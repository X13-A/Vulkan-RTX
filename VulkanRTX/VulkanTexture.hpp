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
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void createTextureSampler(const VulkanContext& context);
    void createTextureImageView(const VulkanContext& context);
    void createTextureImage(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void cleanup(VkDevice device);
};
