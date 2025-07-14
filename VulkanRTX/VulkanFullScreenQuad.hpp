#pragma once
#include "VulkanGeometry.hpp"
#include "VulkanContext.hpp"
#include "VulkanGraphicsPipelineManager.hpp"
#include "VulkanCommandBufferManager.hpp"

struct VulkanFullScreenQuadUBO
{
	float time; // TODO: alignas() ?
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
	void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanGraphicsPipelineManager& graphicsPipeline);
	void createDescriptorSets(const VulkanContext& context, const VulkanGraphicsPipelineManager& graphicsPipeline);
	void writeDescriptorSets(const VulkanContext& context, const VulkanGraphicsPipelineManager& graphicsPipeline);
	void createUniformBuffers(const VulkanContext& context);
	void cleanup(VkDevice device);
};