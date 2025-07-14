#include "VulkanTLAS.hpp"
#include <stdexcept>
#include "VulkanUtils.hpp"
#include <iostream>

void VulkanTLAS::createTLAS(const VulkanContext& context, const std::vector<VulkanModel>& models, VulkanCommandBufferManager& commandBufferManager)
{
    // Fetch BLAS instances
    std::vector<BLASInstance> BLASintances;
    int instanceIndex = 0;
    for (const VulkanModel& model : models)
    {
        BLASInstance instance;
        instance.blas = model.blasHandle;
        instance.transform = model.transform.getTransformMatrix();
        instance.instanceId = instanceIndex++;
        instance.hitGroupIndex = RT_CLOSEST_HIT_GENERAL_SHADER_INDEX;
        BLASintances.push_back(instance);
    }

    createInstanceBuffer(context, BLASintances);

    VkAccelerationStructureBuildSizesInfoKHR buildSizes = getBuildSizes(context, BLASintances.size());

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VulkanUtils::Buffers::createBuffer(context, buildSizes.accelerationStructureSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tlasBuffer, tlasMemory, true);

    VulkanUtils::Buffers::createScratchBuffer(context, buildSizes.buildScratchSize, scratchBuffer, scratchMemory);

    createAccelerationStructure(context, buildSizes.accelerationStructureSize);

    VkCommandBuffer commandBuffer = commandBufferManager.beginSingleTimeCommands(context.device);
    buildTLAS(context, BLASintances.size(), commandBuffer);
    commandBufferManager.endSingleTimeCommands(context.device, context.graphicsQueue, commandBuffer);

    if (debug_vkSetDebugUtilsObjectNameEXT)
    {
        // Name the TLAS for debug
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        nameInfo.objectHandle = (uint64_t)tlas;
        nameInfo.pObjectName = "Scene TLAS";
        debug_vkSetDebugUtilsObjectNameEXT(context.device, &nameInfo);
    }
}

void VulkanTLAS::createInstanceBuffer(const VulkanContext& context, const std::vector<BLASInstance>& instances)
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

        // TODO: use transforms
        VkTransformMatrixKHR identityMatrix = 
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
        };
        memcpy(&instanceData[i].transform, &identityMatrix, sizeof(VkTransformMatrixKHR));

        instanceData[i].instanceCustomIndex = instance.instanceId;
        instanceData[i].mask = 0xFF; // Visible to all rays
        instanceData[i].instanceShaderBindingTableRecordOffset = 0;
        instanceData[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instanceData[i].accelerationStructureReference = blasAddress;
    }

    vkUnmapMemory(context.device, instanceMemory);
}

VkAccelerationStructureBuildSizesInfoKHR VulkanTLAS::getBuildSizes(const VulkanContext& context, uint32_t instanceCount)
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

void VulkanTLAS::createAccelerationStructure(const VulkanContext& context, VkDeviceSize size)
{
    VkAccelerationStructureCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = tlasBuffer;
    createInfo.offset = 0;
    createInfo.size = size;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    rt_vkCreateAccelerationStructureKHR(context.device, &createInfo, nullptr, &tlas);
}

void VulkanTLAS::buildTLAS(const VulkanContext& context, uint32_t instanceCount, VkCommandBuffer commandBuffer)
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
}

uint32_t VulkanTLAS::findMemoryType(const VulkanContext& context, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

VkAccelerationStructureKHR VulkanTLAS::getTLAS() const
{
    return tlas;
}

void VulkanTLAS::cleanup(const VulkanContext& context)
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