#pragma once

#include "Constants.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanGeometry.hpp"

class VulkanUtils
{
public:
    class Hardware
    {
    public:
        static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    };

    class DepthStencil
    {
    public:
        static bool hasStencilComponent(VkFormat format);
        static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
    };

    class Memory
    {
    public:
        static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };

    class Image
    {
    public:
        static void copyBufferToImage(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        static void createImage(const VulkanContext& context, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        static VkImageView createImageView(const VulkanContext& context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
        static void transitionImageLayout(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    };

    class Buffers
    {
    public:
        static void createBuffer(const VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        static void createVertexBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBuffers, const std::vector<VulkanVertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
        static void createIndexBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBuffers, const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
        static void copyBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBuffers, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    };
};