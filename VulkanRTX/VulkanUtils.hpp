#pragma once

#include "Constants.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanGeometry.hpp"


namespace VulkanUtils
{
    namespace Hardware
    {
        VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    };

    namespace Shaders
    {
        VkShaderModule createShaderModule(const VulkanContext& context, const std::vector<char>& code);
    };

    namespace DepthStencil
    {
        bool hasStencilComponent(VkFormat format);
        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
    };

    namespace Memory
    {
        uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };

    namespace Image
    {
        void copyBufferToImage(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void createImage(const VulkanContext& context, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        VkImageView createImageView(const VulkanContext& context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
        void blitImage(
            VkCommandBuffer commandBuffer,
            VkImage srcImage, VkImage dstImage,
            VkFormat srcFormat, VkFormat dstFormat,
            VkAccessFlags srcOriginalAccessMask, VkAccessFlags dstOriginalAccessMask,
            VkImageLayout srcOriginalLayout, VkImageLayout dstOriginalLayout,
            uint32_t srcWidth, uint32_t srcHeight,
            uint32_t dstWidth, uint32_t dstHeight,
            VkFilter filter
        );
        void transition_depthRW_to_depthR_existingCmd(const VulkanContext& context, VkCommandBuffer commandBuffer, VkImage image, VkFormat format);
        void transitionImageLayout(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    };

    namespace Textures
    {
        void createSampler(const VulkanContext& context, VkSampler* sampler, VkFilter minFilter, VkFilter magFilter, VkSamplerMipmapMode mipMapMode);
    };

    namespace Buffers
    {
        void createBuffer(const VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, bool deviceAdressing = false);
        
        template<typename T>
        void createAndFillBuffer(
            const VulkanContext& context,
            VulkanCommandBufferManager& commandBuffers,
            const std::vector<T>& data,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryFlags,
            bool deviceAdressing = false);

        void createScratchBuffer(const VulkanContext& context, VkDeviceSize size, VkBuffer& scratchBuffer, VkDeviceMemory& scratchBufferMemory);
        void copyBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBuffers, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        VkDeviceAddress getBufferDeviceAdress(const VulkanContext& context, VkBuffer buffer);
    };
};

template<typename T>
void VulkanUtils::Buffers::createAndFillBuffer(
    const VulkanContext& context,
    VulkanCommandBufferManager& commandBuffers,
    const std::vector<T>& data,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryFlags,
    bool deviceAdressing)
{
    VkDeviceSize bufferSize = sizeof(T) * data.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // Create staging buffer
    VulkanUtils::Buffers::createBuffer(
        context,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory,
        deviceAdressing
    );

    // Map and copy data
    void* mappedData;
    vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &mappedData);
    memcpy(mappedData, data.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(context.device, stagingBufferMemory);

    VulkanUtils::Buffers::createBuffer(
        context,
        bufferSize,
        usageFlags,
        memoryFlags,
        buffer,
        bufferMemory,
        true
    );

    // Copy from staging to final buffer
    VulkanUtils::Buffers::copyBuffer(context, commandBuffers, stagingBuffer, buffer, bufferSize);

    // Cleanup staging buffer
    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);
}
