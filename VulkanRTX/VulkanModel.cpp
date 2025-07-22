#include "VulkanModel.hpp"
#include "VulkanUtils.hpp"
#include <iostream>
#include "ObjLoader.hpp"
#include "DescriptorSetLayoutManager.hpp"

void VulkanModel::load(ModelInfo info, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorPool descriptorPool)
{
    for (int i = 0; i < info.meshes.size(); ++i)
    {
        ShadedMesh shadedMesh;
        shadedMesh.mesh.vertices = info.meshes[i].vertices;
        shadedMesh.mesh.indices = info.meshes[i].indices;
        shadedMesh.mesh.init(context, commandBufferManager);

        int matIndex = info.meshMaterialIndices[i];
        
        bool hasError = matIndex == -1;
        if (hasError)
        {
            shadedMesh.material.init({}, context, commandBufferManager, descriptorPool, true);
        }
        else
        {
            shadedMesh.material.init(info.materials[matIndex], context, commandBufferManager, descriptorPool, false);
        }
        shadedMeshes.push_back(shadedMesh);
    }

    createUniformBuffers(context);
    createDescriptorSets(context, descriptorPool);
    createBLAS(context, commandBufferManager);
}

void VulkanModel::createDescriptorSets(const VulkanContext& context, VkDescriptorPool descriptorPool)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, DescriptorSetLayoutManager::getModelLayout());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    modelDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    std::cout << "Allocating " << allocInfo.descriptorSetCount << " sets (model)\n";
    if (vkAllocateDescriptorSets(context.device, &allocInfo, modelDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets! (model)");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VulkanModelUBO);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = modelDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanModel::createUniformBuffers(const VulkanContext& context)
{
    VkDeviceSize bufferSize = sizeof(VulkanModelUBO);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VulkanUtils::Buffers::createBuffer(context, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(context.device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VulkanModel::cleanup(VkDevice device)
{
    for (ShadedMesh shadedMesh : shadedMeshes)
    {
        shadedMesh.mesh.cleanup(device);
        shadedMesh.material.cleanup(device);
    }

    rt_vkDestroyAccelerationStructureKHR(device, blasHandle, nullptr);
    blasHandle = VK_NULL_HANDLE;

    vkDestroyBuffer(device, blasBuffer, nullptr);
    vkFreeMemory(device, blasBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
}

void VulkanModel::createBLAS(
    const VulkanContext& context,
    VulkanCommandBufferManager& commandBufferManager)
{
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges;
    std::vector<uint32_t> primitiveCounts;

    // Create geometry for each mesh
    for (const auto& shadedMesh : shadedMeshes)
    {
        const VulkanMesh& mesh = shadedMesh.mesh;

        VkDeviceAddress vertexBufferAddress = VulkanUtils::Buffers::getBufferDeviceAdress(context, mesh.vertexBuffer);
        VkDeviceAddress indexBufferAddress = VulkanUtils::Buffers::getBufferDeviceAdress(context, mesh.indexBuffer);

        VkAccelerationStructureGeometryKHR accelGeometry{};
        accelGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

        // Triangle data
        accelGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        accelGeometry.geometry.triangles.vertexData.deviceAddress = vertexBufferAddress;
        accelGeometry.geometry.triangles.vertexStride = sizeof(VulkanVertex);
        accelGeometry.geometry.triangles.maxVertex = static_cast<uint32_t>(mesh.vertices.size()) - 1;
        accelGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accelGeometry.geometry.triangles.indexData.deviceAddress = indexBufferAddress;
        accelGeometry.geometry.triangles.transformData = {};

        geometries.push_back(accelGeometry);

        // Build range info for this mesh
        uint32_t primitiveCount = static_cast<uint32_t>(mesh.indices.size() / 3);
        primitiveCounts.push_back(primitiveCount);

        VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
        buildRangeInfo.primitiveCount = primitiveCount;
        buildRangeInfo.primitiveOffset = 0;
        buildRangeInfo.firstVertex = 0;
        buildRangeInfo.transformOffset = 0;

        buildRanges.push_back(buildRangeInfo);
    }

    // Build params
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildInfo.geometryCount = static_cast<uint32_t>(geometries.size());
    buildInfo.pGeometries = geometries.data();
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

    // Get required sizes
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
    sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    rt_vkGetAccelerationStructureBuildSizesKHR(
        context.device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        primitiveCounts.data(),
        &sizeInfo);

    // Create scratch buffer
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchBufferMemory;

    VulkanUtils::Buffers::createScratchBuffer(
        context,
        sizeInfo.buildScratchSize,
        scratchBuffer,
        scratchBufferMemory);

    VkDeviceAddress scratchBufferAddress = VulkanUtils::Buffers::getBufferDeviceAdress(context, scratchBuffer);

    // Create BLAS buffer
    VkBufferUsageFlags blasBufferUsage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VulkanUtils::Buffers::createBuffer(
        context,
        sizeInfo.accelerationStructureSize,
        blasBufferUsage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        blasBuffer,
        blasBufferMemory,
        true);

    // Create BLAS
    VkAccelerationStructureCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = blasBuffer;
    createInfo.offset = 0;
    createInfo.size = sizeInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    if (rt_vkCreateAccelerationStructureKHR(context.device, &createInfo, nullptr, &blasHandle) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create BLAS!");
    }

    buildInfo.dstAccelerationStructure = blasHandle;
    buildInfo.scratchData.deviceAddress = scratchBufferAddress;

    // Prepare build range pointers
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> pBuildRangeInfos;
    for (const auto& buildRange : buildRanges)
    {
        pBuildRangeInfos.push_back(&buildRange);
    }

    VkCommandBuffer commandBuffer = commandBufferManager.beginSingleTimeCommands(context.device);
    rt_vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildInfo, pBuildRangeInfos.data());
    commandBufferManager.endSingleTimeCommands(context.device, context.graphicsQueue, commandBuffer);

    // Cleanup scratch buffer
    vkDestroyBuffer(context.device, scratchBuffer, nullptr);
    vkFreeMemory(context.device, scratchBufferMemory, nullptr);

    if (debug_vkSetDebugUtilsObjectNameEXT)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        nameInfo.objectHandle = (uint64_t)blasHandle;
        nameInfo.pObjectName = name.c_str();
        debug_vkSetDebugUtilsObjectNameEXT(context.device, &nameInfo);
    }
}