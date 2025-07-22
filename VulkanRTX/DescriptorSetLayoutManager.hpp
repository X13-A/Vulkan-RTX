#pragma once
#include "Vulkan_GLFW.hpp"
#include "VulkanContext.hpp"

class DescriptorSetLayoutManager
{
private:
	static VkDescriptorSetLayout modelLayout;
	static VkDescriptorSetLayout materialLayout;
	static VkDescriptorSetLayout fullScreenQuadLayout;

public:
	static void createModelLayout(const VulkanContext& context);
	static void createMaterialLayout(const VulkanContext& context);
	static void createFullScreenQuadLayout(const VulkanContext& context);

	static VkDescriptorSetLayout getModelLayout();
	static VkDescriptorSetLayout getMaterialLayout();
	static VkDescriptorSetLayout getFullScreenQuadLayout();

	static void cleanup(VkDevice device);
};