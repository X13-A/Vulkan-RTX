#pragma once

#include "Constants.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include <string>

class VulkanTexture
{
public:
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkSampler sampler;

public:
    void init(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
    void createImageView(const VulkanContext& context, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
    void createImage(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
    void cleanup(VkDevice device);

    static VulkanTexture create1x1TextureRGBA(uint8_t r, uint8_t g, uint8_t b, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
};
