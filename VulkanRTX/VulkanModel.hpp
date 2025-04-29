#pragma once

#include "Constants.hpp"
#include "VulkanGeometry.hpp"
#include "VulkanTexture.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"

#include <vector>

class VulkanModel
{
public:
    std::vector<VulkanVertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VulkanTexture texture;

public:
    void load(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void cleanup(VkDevice device);
};
