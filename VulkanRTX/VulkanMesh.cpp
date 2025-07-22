#include "VulkanMesh.hpp"
#include <tiny_obj_loader.h>
#include <stdexcept>

#include "VulkanUtils.hpp"

void VulkanMesh::init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
    VkBufferUsageFlags vertexUsageFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryPropertyFlags vertexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createAndFillBuffer<VulkanVertex>(context, commandBufferManager, vertices, vertexBuffer, vertexBufferMemory, vertexUsageFlags, vertexMemoryFlags, true);

    VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryPropertyFlags indexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createAndFillBuffer<uint32_t>(context, commandBufferManager, indices, indexBuffer, indexBufferMemory, indexUsageFlags, indexMemoryFlags, true);
}

void VulkanMesh::cleanup(VkDevice device)
{
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vertices.clear();
    indices.clear();
}