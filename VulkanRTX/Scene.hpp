#pragma once
#include <vector>
#include "VulkanModel.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "Vulkan_GLFW.hpp"

struct ModelLoadInfo
{
	std::string name;
	std::string objPath;
	std::string texturePath;
};

class Scene
{
private:
	static std::vector<VulkanModel> loadedModels;
	static const ModelLoadInfo modelLoadInfos[];

public:	
	static uint32_t getModelCount();
	static const std::vector<VulkanModel>& getModels();
	static void loadModels(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool);
	static void update();
	static void cleanup(VkDevice device);
};