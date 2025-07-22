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
    void init(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void createImageView(const VulkanContext& context);
    void createImage(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void cleanup(VkDevice device);

    static VulkanTexture create1x1Texture(float r, float g, float b, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
};
