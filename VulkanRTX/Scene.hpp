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
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;
};

class Scene
{
private:
	static std::vector<VulkanModel> models; // The loaded models
	static std::vector<ModelInfo> modelInfos; // Information on models, fetched at runtime
	static const ModelLoadInfo modelLoadInfos[]; // Configuration to load model files

public:	
	static uint32_t getModelCount();
	static uint32_t getMaterialCount();
	static uint32_t getMeshCount();
	static const std::vector<VulkanModel>& getModels();
	static void fetchModels();
	static void loadModels(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorPool descriptorPool);
	static void update();
	static void cleanup(VkDevice device);
};