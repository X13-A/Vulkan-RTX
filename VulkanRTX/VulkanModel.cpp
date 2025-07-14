#include "VulkanModel.hpp"
#include "VulkanUtils.hpp"
#include <iostream>

#ifndef TINYOBJLOADER_IMPLEMENTATION
    #define TINYOBJLOADER_IMPLEMENTATION
    #include <tiny_obj_loader.h>
#endif


void VulkanModel::loadObj(std::string objPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    // Load the model
    std::unordered_map<VulkanVertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            VulkanVertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void VulkanModel::init(std::string objPath, std::string texturePath, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool)
{
    loadObj(objPath);
    
    VkBufferUsageFlags vertexUsageFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryPropertyFlags vertexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createVertexBuffer(context, commandBufferManager, vertices, vertexBuffer, vertexBufferMemory, vertexUsageFlags, vertexMemoryFlags, true);

    VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryPropertyFlags indexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createIndexBuffer(context, commandBufferManager, indices, indexBuffer, indexBufferMemory, indexUsageFlags, indexMemoryFlags, true);

    albedoTexture.init(texturePath, context, commandBufferManager);
    createUniformBuffers(context);
    createDescriptorSets(context, geometryDescriptorSetLayout, descriptorPool);
    createBLAS(context, commandBufferManager);
}

void VulkanModel::createDescriptorSets(const VulkanContext& context, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, geometryDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(context.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VulkanModelUBO);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = albedoTexture.textureImageView;
        imageInfo.sampler = albedoTexture.textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

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
    albedoTexture.cleanup(device);

    rt_vkDestroyAccelerationStructureKHR(device, blasHandle, nullptr);
    blasHandle = VK_NULL_HANDLE;

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
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
    VkDeviceAddress vertexBufferAddress = VulkanUtils::Buffers::getBufferDeviceAdress(context, vertexBuffer);
    VkDeviceAddress indexBufferAddress = VulkanUtils::Buffers::getBufferDeviceAdress(context, indexBuffer);

    // Define geometry
    VkAccelerationStructureGeometryKHR accelGeometry{};
    accelGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    accelGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    accelGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    // Define triangles data
    accelGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    accelGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    accelGeometry.geometry.triangles.vertexData.deviceAddress = vertexBufferAddress;
    accelGeometry.geometry.triangles.vertexStride = sizeof(VulkanVertex);
    accelGeometry.geometry.triangles.maxVertex = static_cast<uint32_t>(vertices.size()) - 1;
    accelGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    accelGeometry.geometry.triangles.indexData.deviceAddress = indexBufferAddress;
    accelGeometry.geometry.triangles.transformData.deviceAddress = 0;
    accelGeometry.geometry.triangles.transformData.hostAddress = nullptr;
    accelGeometry.geometry.triangles.transformData = {};

    // Configure construction parameters
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &accelGeometry;
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

    // Get required sizes
    uint32_t primitiveCount = static_cast<uint32_t>(indices.size() / 3);

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
    sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    rt_vkGetAccelerationStructureBuildSizesKHR(
        context.device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        &primitiveCount,
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

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = primitiveCount;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;

    const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildRangeInfo;
    VkCommandBuffer commandBuffer = commandBufferManager.beginSingleTimeCommands(context.device);
    rt_vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildInfo, &pBuildRangeInfo);
    commandBufferManager.endSingleTimeCommands(context.device, context.graphicsQueue, commandBuffer);

    // Cleanup scratch buffer
    vkDestroyBuffer(context.device, scratchBuffer, nullptr);
    vkFreeMemory(context.device, scratchBufferMemory, nullptr);

    if (debug_vkSetDebugUtilsObjectNameEXT)
    {
        // Name the BLAS for debug
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        nameInfo.objectHandle = (uint64_t)blasHandle;
        nameInfo.pObjectName = name.c_str();
        debug_vkSetDebugUtilsObjectNameEXT(context.device, &nameInfo);
    }
}