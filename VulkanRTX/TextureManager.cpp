#include "TextureManager.hpp"

VulkanTexture TextureManager::errorAlbedoTexture = {};
VulkanTexture TextureManager::errorBumpTexture = {};

void TextureManager::loadTextures(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
	errorAlbedoTexture.init("textures/error/albedo.png", context, commandBufferManager);
	errorBumpTexture.init("textures/error/normal.png", context, commandBufferManager);
}

void TextureManager::cleanup(VkDevice device)
{
	errorAlbedoTexture.cleanup(device);
	errorBumpTexture.cleanup(device);
}
