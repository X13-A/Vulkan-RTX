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
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkFormat swapChainImageFormat);
    void cleanup(VkDevice device);
    void handleResize(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void recordDrawCommands(int width, int height, const VulkanSwapChainManager& swapChainManager, const VulkanFullScreenQuad& fullScreenQuad, VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t imageIndex);
    VkRenderPass getRenderPass() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

private:
    void createRenderPasses(const VulkanContext& context, VkFormat swapChainImageFormat);
    void createPipelineLayouts(const VulkanContext& context);
    void createPipeline(const VulkanContext& context);
};