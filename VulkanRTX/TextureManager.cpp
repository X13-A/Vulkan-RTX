#include "TextureManager.hpp"

VulkanTexture TextureManager::errorTexture = {};

void TextureManager::loadTextures(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
	errorTexture.init("textures/error.png", context, commandBufferManager);
}

void TextureManager::cleanup(VkDevice device)
{
	errorTexture.cleanup(device);
}
