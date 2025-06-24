#pragma once

#include <vector>
#include "Constants.hpp"
#include "Vulkan_GLFW.hpp"
#include "VulkanContext.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanModel.hpp"
#include "VulkanFullScreenQuad.hpp"
#include "Camera.hpp"

class VulkanRenderer
{
public:
    uint32_t currentFrame = 0;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

public:
    void createSyncObjects(const VulkanContext& context, const VulkanSwapChainManager& swapChainManager);
    void recordCommandBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager, const VulkanGraphicsPipeline& graphicsPipeline, uint32_t imageIndex, uint32_t currentFrame, const std::vector<VulkanModel>& models, const VulkanFullScreenQuad& fullScreenQuad);
    void handleResize(GLFWwindow* window, const VulkanContext& context, VulkanSwapChainManager& swapChainManager, VulkanGraphicsPipeline& graphicsPipeline, VulkanCommandBufferManager& commandBufferManager, VulkanFullScreenQuad& fullScreenQuad);
    void drawFrame(GLFWwindow* window, const VulkanContext& context, VulkanSwapChainManager& swapChainManager, VulkanGraphicsPipeline& graphicsPipeline, VulkanCommandBufferManager& commandBufferManager, const Camera& camera, const std::vector<VulkanModel>& models, VulkanFullScreenQuad& fullScreenQuad);
    void updateUniformBuffers(const Camera& camera, const std::vector<VulkanModel>& models, const VulkanFullScreenQuad& fullScreenQuad, const VulkanSwapChainManager& swapChain, uint32_t currentImage);
    void cleanup(VkDevice device);
};