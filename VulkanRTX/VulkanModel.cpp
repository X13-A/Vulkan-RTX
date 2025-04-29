#include "VulkanModel.hpp"
#include <iostream>

#ifndef TINYOBJLOADER_IMPLEMENTATION
    #define TINYOBJLOADER_IMPLEMENTATION
    #include <tiny_obj_loader.h>
#endif
#include "VulkanUtils.hpp"

void VulkanModel::load(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
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

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }

    // Create buffers
    VulkanUtils::Buffers::createVertexBuffer(context, commandBufferManager, vertices, vertexBuffer, vertexBufferMemory);
    VulkanUtils::Buffers::createIndexBuffer(context, commandBufferManager, indices, indexBuffer, indexBufferMemory);

}

void VulkanModel::cleanup(VkDevice device)
{
    texture.cleanup(device);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}