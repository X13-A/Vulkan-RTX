#pragma once

#include "Constants.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"

class VulkanTexture
{
public:
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

public:
    void createTextureSampler(const VulkanContext& context);
    void createTextureImageView(const VulkanContext& context);
    void createTextureImage(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
};
