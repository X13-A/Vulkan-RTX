#pragma once

#include "VulkanTexture.hpp"
#include "GLM_defines.hpp"
#include "Vulkan_GLFW.hpp"
#include "Constants.hpp"
#include "VulkanContext.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanGBufferManager.hpp"

#include <vector>

struct VulkanFullScreenQuadUBO
{
    float time; // TODO: alignas() ?
};

struct VulkanModelUBO
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class VulkanGraphicsPipeline
{
public:
    VkDescriptorSetLayout geometryDescriptorSetLayout;
    VkDescriptorSetLayout lightingDescriptorSetLayout;

    VkDescriptorPool descriptorPool;

    VkRenderPass geometryRenderPass;
    VkRenderPass lightingRenderPass;

    VkFramebuffer geometryFramebuffer;
    // No lighting framebuffer, it renders directly to the swapchain

    VkPipelineLayout geometryPipelineLayout;
    VkPipelineLayout lightingPipelineLayout;

    VkPipeline geometryPipeline;
    VkPipeline lightingPipeline;


    VulkanGBufferManager gBufferManager;

public:
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager);
    void createDescriptorSetLayouts(const VulkanContext& context);
    void createDescriptorPool(const VulkanContext& context, size_t modelCount, size_t fullScreenQuadCount);
    void createRenderPasses(const VulkanContext& context, VkFormat swapChainImageFormat);
    void createFramebuffers(const VulkanContext& context, uint32_t width, uint32_t height);
    void createPipelineLayouts(const VulkanContext& context);
    void createPipelines(const VulkanContext& context, const VulkanSwapChainManager& swapChain);
    VkShaderModule createShaderModule(const VulkanContext& context, const std::vector<char>& code);
    void handleResize(GLFWwindow* window, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager);
    void cleanup(VkDevice device);
};
