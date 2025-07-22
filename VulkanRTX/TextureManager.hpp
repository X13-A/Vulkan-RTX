#pragma once
#include "VulkanTexture.hpp"

class TextureManager
{
public:
	static VulkanTexture errorTexture;
	static void loadTextures(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
	static void cleanup(VkDevice device);
};