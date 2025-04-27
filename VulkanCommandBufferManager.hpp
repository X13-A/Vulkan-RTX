#pragma once
#include <vector>
#include "VulkanContext.hpp"
#include "Constants.hpp"

#include "Vulkan_GLFW.hpp"

class VulkanCommandBufferManager
{
public:
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

public:
    VkCommandBuffer beginSingleTimeCommands(VkDevice device) const;
    void endSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
    void createCommandBuffers(const VulkanContext& context);
    void createCommandPool(const VulkanContext& context);
};
