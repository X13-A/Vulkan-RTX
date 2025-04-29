#pragma once

#include <vector>
#include "VulkanContext.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanModel.hpp"
#include "Constants.hpp"
#include "Vulkan_GLFW.hpp"

class VulkanRenderer
{
public:
    uint32_t currentFrame = 0;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

public:
    void createSyncObjects(const VulkanContext& context);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, const VulkanSwapChainManager& swapChainManager, const VulkanGraphicsPipeline& graphicsPipeline, uint32_t imageIndex, const VulkanModel& model);
    void drawFrame(GLFWwindow* window, const VulkanContext& context, VulkanSwapChainManager& swapChainManager, const VulkanGraphicsPipeline& graphicsPipeline, VulkanCommandBufferManager& commandBufferManager, const VulkanModel& model);
    void updateUniformBuffer(const VulkanGraphicsPipeline& graphicsPipeline, const VulkanSwapChainManager& swapChain, uint32_t currentImage);
    void cleanup(VkDevice device);
};