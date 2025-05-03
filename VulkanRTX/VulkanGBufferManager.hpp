#pragma once

#include "VulkanUtils.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include <vulkan/vulkan.h>

class VulkanGBufferManager
{
public:
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage normalImage;
    VkDeviceMemory normalImageMemory;
    VkImageView normalImageView;

    VkImage albedoImage;
    VkDeviceMemory albedoImageMemory;
    VkImageView albedoImageView;

public:
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height);
    void createDepthResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height);
    void createNormalResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height);
    void createAlbedoResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, size_t width, size_t height);
    void cleanup(VkDevice device);
};
