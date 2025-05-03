#pragma once
#include "VulkanGeometry.hpp"
#include "VulkanContext.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanCommandBufferManager.hpp"

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
	void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanGraphicsPipeline& graphicsPipeline);
	void createDescriptorSets(const VulkanContext& context, const VulkanGraphicsPipeline& graphicsPipeline);
	void writeDescriptorSets(const VulkanContext& context, const VulkanGraphicsPipeline& graphicsPipeline);
	void createUniformBuffers(const VulkanContext& context);
	void cleanup(VkDevice device);
};