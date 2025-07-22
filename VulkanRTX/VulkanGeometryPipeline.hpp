#pragma once
#include "Vulkan_GLFW.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanGBufferManager.hpp"
#include "VulkanModel.hpp"

class VulkanGeometryPipeline
{
private:
    VkRenderPass renderPass;
    VkFramebuffer framebuffer;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

public:
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager, const VulkanGBufferManager& gBufferManager);
    void cleanup(VkDevice device);
    void handleResize(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager, const VulkanGBufferManager& gBufferManager, uint32_t width, uint32_t height);
    void recordDrawCommands(const VulkanSwapChainManager& swapChainManager, const std::vector<VulkanModel>& models, VkCommandBuffer commandBuffer, uint32_t currentFrame);

    VkRenderPass getRenderPass() const;
    VkFramebuffer getFrameBuffer() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
private:
    void createDescriptorPool(const VulkanContext& context, size_t modelCount, size_t fullScreenQuadCount);
    void createRenderPasses(const VulkanContext& context, VkFormat swapChainImageFormat);
    void createFramebuffers(const VulkanContext& context, const VulkanGBufferManager& gBufferManager, uint32_t width, uint32_t height);
    void createPipelineLayouts(const VulkanContext& context);
    void createPipeline(const VulkanContext& context, const VulkanSwapChainManager& swapChain);
};