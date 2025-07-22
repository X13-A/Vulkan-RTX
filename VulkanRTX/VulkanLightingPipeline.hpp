#pragma once
#include "Vulkan_GLFW.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanModel.hpp"
#include "VulkanFullScreenQuad.hpp"

class VulkanLightingPipeline
{
private:
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

public:
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager);
    void cleanup(VkDevice device);
    void handleResize(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager);
    void recordDrawCommands(const VulkanSwapChainManager& swapChainManager, const VulkanFullScreenQuad& fullScreenQuad, VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t imageIndex);
    VkRenderPass getRenderPass() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

private:
    void createDescriptorPool(const VulkanContext& context, size_t modelCount, size_t fullScreenQuadCount);
    void createRenderPasses(const VulkanContext& context, VkFormat swapChainImageFormat);
    void createPipelineLayouts(const VulkanContext& context);
    void createPipeline(const VulkanContext& context, const VulkanSwapChainManager& swapChain);
};