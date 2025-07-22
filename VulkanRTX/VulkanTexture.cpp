#include "VulkanTexture.hpp"
#include "VulkanUtils.hpp"
#include <iostream>
#include <vector>

// One time stb_image implementation
#ifndef STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>
#endif

void VulkanTexture::init(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
    createImage(path, context, commandBufferManager);
    createImageView(context);

    // TODO: don't create a sampler everytime, reuse a sampler instead
    VulkanUtils::Textures::createSampler(context, &sampler);
}

void VulkanTexture::createImageView(const VulkanContext& context)
{
    imageView = VulkanUtils::Image::createImageView(context, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanTexture::createImage(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VulkanUtils::Buffers::createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context.device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context.device, stagingBufferMemory);

    stbi_image_free(pixels);

    VulkanUtils::Image::createImage(context, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

    VulkanUtils::Image::transitionImageLayout(context, commandBufferManager, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VulkanUtils::Image::copyBufferToImage(context, commandBufferManager, stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    VulkanUtils::Image::transitionImageLayout(context, commandBufferManager, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);
}

void VulkanTexture::cleanup(VkDevice device)
{
    vkDestroySampler(device, sampler, nullptr);
    vkDestroyImageView(device, imageView, nullptr);
    vkDestroyImage(device, image, nullptr);
    vkFreeMemory(device, imageMemory, nullptr);
}

VulkanTexture VulkanTexture::create1x1Texture(float r, float g, float b, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
    VulkanTexture texture;

    // Create 1x1 pixel data
    std::array<uint8_t, 4> pixelData = {
        static_cast<uint8_t>(r * 255.0f),
        static_cast<uint8_t>(g * 255.0f),
        static_cast<uint8_t>(b * 255.0f),
        255
    };

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VulkanUtils::Buffers::createBuffer(
        context,
        sizeof(pixelData),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory,
        false
    );

    // Copy pixel data to staging buffer
    void* data;
    vkMapMemory(context.device, stagingBufferMemory, 0, sizeof(pixelData), 0, &data);
    memcpy(data, pixelData.data(), sizeof(pixelData));
    vkUnmapMemory(context.device, stagingBufferMemory);

    // Create 1x1 image
    VulkanUtils::Image::createImage(
        context,
        1, 1,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texture.image,
        texture.imageMemory
    );

    // Transition to transfer destination
    VulkanUtils::Image::transitionImageLayout(
        context,
        commandBufferManager,
        texture.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    // Copy buffer to image
    VulkanUtils::Image::copyBufferToImage(
        context,
        commandBufferManager,
        stagingBuffer,
        texture.image,
        1, 1
    );

    // Transition to shader read only
    VulkanUtils::Image::transitionImageLayout(
        context,
        commandBufferManager,
        texture.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    // Create image view
    texture.imageView = VulkanUtils::Image::createImageView(
        context,
        texture.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    // Clean up staging buffer
    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);

    // Create a sampler
    VulkanUtils::Textures::createSampler(context, &texture.sampler);
    return texture;
}