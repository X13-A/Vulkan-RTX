#pragma once

#include "Constants.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include <string>

class VulkanTexture
{
public:
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

public:
    void init(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void createTextureSampler(const VulkanContext& context);
    void createTextureImageView(const VulkanContext& context);
    void createTextureImage(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void cleanup(VkDevice device);
};
