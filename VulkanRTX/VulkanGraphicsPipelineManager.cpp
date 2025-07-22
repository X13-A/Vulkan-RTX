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

void VulkanGraphicsPipelineManager::createDescriptorPool(const VulkanContext& context, size_t modelCount, size_t meshCount, size_t fullScreenQuadCount)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    // Uniform buffer descriptors
    // - 1 per model
    // - 1 per fullscreen quad
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>((modelCount + fullScreenQuadCount) * MAX_FRAMES_IN_FLIGHT);

    // Combined image sampler descriptors
    // - 1 per material (albedo texture) -> Use meshCount because a mesh 100% has a material (can be a fallback)
    // - TODO: add normals, smoothness, etc
    // - 3 per fullscreen quad (G-Buffer textures: depth, normal, albedo)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>((meshCount + fullScreenQuadCount * 3) * MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    // Total descriptor sets needed:
    // - 1 per model (geometry/transform)
    // - 1 per material (textures) 
    // - 1 per fullscreen quad (lighting)
    // TODO: problem here !
    poolInfo.maxSets = static_cast<uint32_t>((modelCount + meshCount + fullScreenQuadCount) * MAX_FRAMES_IN_FLIGHT);
    
    std::cout << "Creating descriptor pool with " << poolInfo.maxSets << " max sets" << std::endl;
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
