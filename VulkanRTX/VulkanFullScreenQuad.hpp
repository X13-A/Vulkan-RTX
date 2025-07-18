#pragma once
#include "VulkanGeometry.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"

struct VulkanFullScreenQuadUBO
{
	float time;
};

class VulkanFullScreenQuad
{
public:
	std::vector<VulkanVertex> vertices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkDescriptorSet> descriptorSets;
	VkSampler gBufferSampler;

public:
	void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorSetLayout layout, VkDescriptorPool pool, VkImageView depthImageView, VkImageView normalImageView, VkImageView albedoImageView);
	void createDescriptorSets(const VulkanContext& context, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool);
	void writeDescriptorSets(const VulkanContext& context, VkImageView depthImageView, VkImageView normalImageView, VkImageView albedoImageView);
	void createUniformBuffers(const VulkanContext& context);
	void cleanup(VkDevice device);
};