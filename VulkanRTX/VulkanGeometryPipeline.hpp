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

    int width, height;

public:
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, int width, int height, const VulkanGBufferManager& gBufferManager);
    void cleanup(VkDevice device);
    void handleResize(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanGBufferManager& gBufferManager, int width, int height);
    void recordDrawCommands(int width, int height, const std::vector<VulkanModel>& models, VkCommandBuffer commandBuffer, uint32_t currentFrame);

    VkRenderPass getRenderPass() const;
    VkFramebuffer getFrameBuffer() const;
    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
private:
    void createRenderPasses(const VulkanContext& context);
    void createFramebuffers(const VulkanContext& context, const VulkanGBufferManager& gBufferManager, uint32_t width, uint32_t height);
    void createPipelineLayouts(const VulkanContext& context);
    void createPipeline(const VulkanContext& context);
};