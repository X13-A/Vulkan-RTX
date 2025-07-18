#include "VulkanGraphicsPipelineManager.hpp"
#include <array>
#include <iostream>
#include "RunTimeSettings.hpp"

void VulkanGraphicsPipelineManager::initPipelines(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager)
{
    gBufferManager.init(context, commandBufferManager, swapChainManager.swapChainExtent.width, swapChainManager.swapChainExtent.height);
    geometryPipeline.init(context, commandBufferManager, swapChainManager, gBufferManager);
    lightingPipeline.init(context, commandBufferManager, swapChainManager);
    rtPipeline.init(context, swapChainManager.swapChainExtent.width, swapChainManager.swapChainExtent.height);
}

void VulkanGraphicsPipelineManager::createDescriptorPool(const VulkanContext& context, size_t modelCount, size_t fullScreenQuadCount)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(modelCount * MAX_FRAMES_IN_FLIGHT + fullScreenQuadCount * MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(modelCount * MAX_FRAMES_IN_FLIGHT + fullScreenQuadCount * 3 * MAX_FRAMES_IN_FLIGHT); // * 3 because 3 textures in G-Buffer

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(modelCount * MAX_FRAMES_IN_FLIGHT + fullScreenQuadCount * MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanGraphicsPipelineManager::handleResize(GLFWwindow* window, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context.device);

    // GBuffer
    gBufferManager.cleanup(context.device);
    gBufferManager.init(context, commandBufferManager, width, height);

    geometryPipeline.handleResize(context, commandBufferManager, swapChainManager, gBufferManager, width, height);
    lightingPipeline.handleResize(context, commandBufferManager, swapChainManager);
    
    rtPipeline.handleResize(context, width, height, gBufferManager.depthImageView, gBufferManager.normalImageView, gBufferManager.albedoImageView);
}

void VulkanGraphicsPipelineManager::cleanup(VkDevice device)
{
    rtPipeline.cleanup(device);
    geometryPipeline.cleanup(device);
    lightingPipeline.cleanup(device);
    gBufferManager.cleanup(device);

    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
}
