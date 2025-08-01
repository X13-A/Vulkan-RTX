#pragma once

#include "Vulkan_GLFW.hpp"
#include "VulkanContext.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanGBufferManager.hpp"
#include "VulkanRayTracingPipeline.hpp"
#include "VulkanGeometryPipeline.hpp"
#include "VulkanLightingPipeline.hpp"

class VulkanGraphicsPipelineManager
{
public:
    VkDescriptorPool descriptorPool;

    VulkanGeometryPipeline geometryPipeline;
    VulkanLightingPipeline lightingPipeline;
    VulkanRayTracingPipeline rtPipeline;

    // TODO: remove gbuffer from this class, not a gfx pipeline
    VulkanGBufferManager gBufferManager;

public:
    void initPipelines(int nativeWidth, int nativeHeight, int scaledWidth, int scaledHeight, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkFormat swapChainImageFormat);
    void createDescriptorPool(const VulkanContext& context, size_t modelCount, size_t materialCount, size_t fullScreenQuadCount);
    void handleResize(int nativeWidth, int nativeHeight, int scaledWidth, int scaledHeight, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void cleanup(VkDevice device);
};
