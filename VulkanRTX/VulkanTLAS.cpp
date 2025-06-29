#include "VulkanTLAS.hpp"
#include <stdexcept>
#include "VulkanUtils.hpp"

VkAccelerationStructureKHR TLASBuilder::createTLAS(const std::vector<BLASInstance>& instances, VkCommandBuffer commandBuffer)
{
    createInstanceBuffer(instances);

    VkAccelerationStructureBuildSizesInfoKHR buildSizes = getBuildSizes(instances.size());

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VulkanUtils::Buffers::createBuffer(context, buildSizes.accelerationStructureSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tlasBuffer, tlasMemory, true);

    VulkanUtils::Buffers::createScratchBuffer(context, buildSizes.buildScratchSize, scratchBuffer, scratchMemory);

    createAccelerationStructure(buildSizes.accelerationStructureSize);

    buildTLAS(instances.size(), commandBuffer);

    VkAccelerationStructureKHR result = tlas;
    tlas = VK_NULL_HANDLE;
    return result;
}

void TLASBuilder::createInstanceBuffer(const std::vector<BLASInstance>& instances)
{
    VkDeviceSize bufferSize = sizeof(VkAccelerationStructureInstanceKHR) * instances.size();
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VulkanUtils::Buffers::createBuffer(context, bufferSize, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceBuffer, instanceMemory, true);
    
    // Fill instance data
    VkAccelerationStructureInstanceKHR* instanceData;
    vkMapMemory(context.device, instanceMemory, 0, bufferSize, 0, (void**)&instanceData);

    for (size_t i = 0; i < instances.size(); i++)
    {
        const auto& instance = instances[i];

        // Get BLAS device address
        VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
        addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        addressInfo.accelerationStructure = instance.blas;
        VkDeviceAddress blasAddress = rt_vkGetAccelerationStructureDeviceAddressKHR(context.device, &addressInfo);

        // Convert glm::mat4 to VkTransformMatrixKHR
        // TODO: double check this
        glm::mat4 transposed = glm::transpose(instance.transform);
        memcpy(&instanceData[i].transform, &transposed, sizeof(VkTransformMatrixKHR));

        instanceData[i].instanceCustomIndex = instance.instanceId;
        instanceData[i].mask = 0xFF; // Visible to all rays
        instanceData[i].instanceShaderBindingTableRecordOffset = instance.hitGroupIndex;
        instanceData[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instanceData[i].accelerationStructureReference = blasAddress;
    }

    vkUnmapMemory(context.device, instanceMemory);
}

VkAccelerationStructureBuildSizesInfoKHR TLASBuilder::getBuildSizes(uint32_t instanceCount)
{
    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometry.geometry.instances.arrayOfPointers = VK_FALSE;

    VkBufferDeviceAddressInfo addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = instanceBuffer;
    geometry.geometry.instances.data.deviceAddress = rt_vkGetBufferDeviceAddressKHR(context.device, &addressInfo);

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry;

    VkAccelerationStructureBuildSizesInfoKHR buildSizes{};
    buildSizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    rt_vkGetAccelerationStructureBuildSizesKHR(context.device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        &instanceCount,
        &buildSizes);

    return buildSizes;
}

void TLASBuilder::createAccelerationStructure(VkDeviceSize size)
{
    VkAccelerationStructureCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = tlasBuffer;
    createInfo.offset = 0;
    createInfo.size = size;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    rt_vkCreateAccelerationStructureKHR(context.device, &createInfo, nullptr, &tlas);
}

void TLASBuilder::buildTLAS(uint32_t instanceCount, VkCommandBuffer commandBuffer)
{
    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometry.geometry.instances.arrayOfPointers = VK_FALSE;

    VkBufferDeviceAddressInfo addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = instanceBuffer;
    geometry.geometry.instances.data.deviceAddress = rt_vkGetBufferDeviceAddressKHR(context.device, &addressInfo);

    VkBufferDeviceAddressInfo scratchAddressInfo{};
    scratchAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    scratchAddressInfo.buffer = scratchBuffer;
    VkDeviceAddress scratchAddress = rt_vkGetBufferDeviceAddressKHR(context.device, &scratchAddressInfo);

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.dstAccelerationStructure = tlas;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry;
    buildInfo.scratchData.deviceAddress = scratchAddress;

    VkAccelerationStructureBuildRangeInfoKHR buildRange{};
    buildRange.primitiveCount = instanceCount;
    buildRange.primitiveOffset = 0;
    buildRange.firstVertex = 0;
    buildRange.transformOffset = 0;

    const VkAccelerationStructureBuildRangeInfoKHR* pBuildRange = &buildRange;

    rt_vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildInfo, &pBuildRange);

    // Memory barrier
    VkMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
        0, 1, &barrier, 0, nullptr, 0, nullptr);
}

uint32_t TLASBuilder::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void TLASBuilder::destroyTLAS(const VulkanContext& context, VkAccelerationStructureKHR& tlas)
{
    if (tlas != VK_NULL_HANDLE) 
    {
        rt_vkDestroyAccelerationStructureKHR(context.device, tlas, nullptr);
        tlas = VK_NULL_HANDLE;
    }
}

void TLASBuilder::cleanup()
{
    // Acceleration structure
    if (tlas != VK_NULL_HANDLE) 
    {
        rt_vkDestroyAccelerationStructureKHR(context.device, tlas, nullptr);
        tlas = VK_NULL_HANDLE;
    }

    // TLAS buffer
    if (tlasBuffer != VK_NULL_HANDLE) 
    {
        vkDestroyBuffer(context.device, tlasBuffer, nullptr);
        tlasBuffer = VK_NULL_HANDLE;
    }
    if (tlasMemory != VK_NULL_HANDLE) 
    {
        vkFreeMemory(context.device, tlasMemory, nullptr);
        tlasMemory = VK_NULL_HANDLE;
    }

    // Instance buffer
    if (instanceBuffer != VK_NULL_HANDLE) 
    {
        vkDestroyBuffer(context.device, instanceBuffer, nullptr);
        instanceBuffer = VK_NULL_HANDLE;
    }
    if (instanceMemory != VK_NULL_HANDLE) 
    {
        vkFreeMemory(context.device, instanceMemory, nullptr);
        instanceMemory = VK_NULL_HANDLE;
    }

    // Scratch buffer
    if (scratchBuffer != VK_NULL_HANDLE) 
    {
        vkDestroyBuffer(context.device, scratchBuffer, nullptr);
        scratchBuffer = VK_NULL_HANDLE;
    }
    if (scratchMemory != VK_NULL_HANDLE) 
    {
        vkFreeMemory(context.device, scratchMemory, nullptr);
        scratchMemory = VK_NULL_HANDLE;
    }
}