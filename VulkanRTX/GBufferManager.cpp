#include "VulkanGBufferManager.hpp"
#include "VulkanUtils.hpp"

void VulkanGBufferManager::init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height)
{
    createDepthResources(context, commandBufferManager, width, height);
    createNormalResources(context, commandBufferManager, width, height);
    createAlbedoResources(context, commandBufferManager, width, height);
}

void VulkanGBufferManager::createDepthResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height)
{
    // Depth format for the G-buffer (use a format that supports depth-stencil)
    VkFormat depthFormat = VulkanUtils::DepthStencil::findDepthFormat(context.physicalDevice);

    // Create the depth image
    VulkanUtils::Image::createImage(
        context,
        width,
        height,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage,
        depthImageMemory
    );

    // Create image view for the depth image
    depthImageView = VulkanUtils::Image::createImageView(
        context,
        depthImage,
        depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    // Transition the depth image to the appropriate layout (depth-stencil attachment)
    VulkanUtils::Image::transitionImageLayout(
        context,
        commandBufferManager,
        depthImage,
        depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
}

void VulkanGBufferManager::createNormalResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height)
{
    // Format for the normal image (use a format that supports RGBA color channels)
    VkFormat normalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

    // Create the normal image
    VulkanUtils::Image::createImage(
        context,
        width,
        height,
        normalFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        normalImage,
        normalImageMemory
    );

    // Create image view for the normal image
    normalImageView = VulkanUtils::Image::createImageView(
        context,
        normalImage,
        normalFormat,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    // Transition the normal image to the appropriate layout (color attachment)
    VulkanUtils::Image::transitionImageLayout(
        context,
        commandBufferManager,
        normalImage,
        normalFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
}

void VulkanGBufferManager::createAlbedoResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height)
{
    VkFormat albedoFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

    VulkanUtils::Image::createImage(
        context,
        width,
        height,
        albedoFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        albedoImage,
        albedoImageMemory
    );

    albedoImageView = VulkanUtils::Image::createImageView(
        context,
        albedoImage,
        albedoFormat,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    VulkanUtils::Image::transitionImageLayout(
        context,
        commandBufferManager,
        albedoImage,
        albedoFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
}

void VulkanGBufferManager::cleanup(VkDevice device)
{
    vkDestroyImageView(device, depthImageView, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    vkDestroyImage(device, depthImage, nullptr);

    vkDestroyImageView(device, normalImageView, nullptr);
    vkFreeMemory(device, normalImageMemory, nullptr);
    vkDestroyImage(device, normalImage, nullptr);

    vkDestroyImageView(device, albedoImageView, nullptr);
    vkFreeMemory(device, albedoImageMemory, nullptr);
    vkDestroyImage(device, albedoImage, nullptr);
}
