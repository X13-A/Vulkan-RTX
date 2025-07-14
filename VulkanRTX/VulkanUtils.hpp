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

    class Shaders
    {
    public:
        static VkShaderModule createShaderModule(const VulkanContext& context, const std::vector<char>& code);
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
        static void blitImage(
            VkCommandBuffer commandBuffer,
            VkImage srcImage, VkImage dstImage,
            VkFormat srcFormat, VkFormat dstFormat,
            VkAccessFlags srcOriginalAccessMask, VkAccessFlags dstOriginalAccessMask,
            VkImageLayout srcOriginalLayout, VkImageLayout dstOriginalLayout,
            uint32_t srcWidth, uint32_t srcHeight,
            uint32_t dstWidth, uint32_t dstHeight,
            VkFilter filter
        );
        static void transition_depthRW_to_depthR_existingCmd(const VulkanContext& context, VkCommandBuffer commandBuffer, VkImage image, VkFormat format);
        static void transitionImageLayout(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    };

    class Textures
    {
    public:
        static void createSampler(const VulkanContext& context, VkSampler* sampler);
    };

    class Buffers
    {
    public:
        static void createBuffer(const VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, bool deviceAdressing = false);
        static void createVertexBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBuffers, const std::vector<VulkanVertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, bool deviceAdressing = false);
        static void createIndexBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBuffers, const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, bool deviceAdressing = false);
        static void createScratchBuffer(const VulkanContext& context, VkDeviceSize size, VkBuffer& scratchBuffer, VkDeviceMemory& scratchBufferMemory);
        static void copyBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBuffers, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        static VkDeviceAddress getBufferDeviceAdress(const VulkanContext& context, VkBuffer buffer);
    };
};