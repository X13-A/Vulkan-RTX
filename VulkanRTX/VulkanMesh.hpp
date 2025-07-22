#pragma once
#include <vector>
#include "VulkanGeometry.hpp"
#include <string>
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanMaterial.hpp"

class VulkanMesh
{
public:
    std::vector<VulkanVertex> vertices;
    std::vector<uint32_t> indices;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

public:
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void cleanup(VkDevice device);
};