#pragma once
#include "VulkanTexture.hpp"
#include "ObjLoader.hpp"

class VulkanMaterial
{ 
public:
	VulkanTexture albedoTexture;
	std::vector<VkDescriptorSet> descriptorSets;
	bool hasError = false;

	void init(const PBRMaterialInfo& info, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorPool descriptorPool, bool hasError);
	static VkDescriptorSetLayout createDescriptorSetLayout(const VulkanContext& context);
	void createDescriptorSets(const VulkanContext& context, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool);
	void cleanup(VkDevice device);
};