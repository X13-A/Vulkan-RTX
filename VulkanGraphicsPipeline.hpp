#pragma once

#include "Constants.hpp"
#include "VulkanTexture.hpp"
#include "GLM_defines.hpp"

#include <vector>

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class VulkanGraphicsPipeline
{
public:
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

public:
    void createDescriptorSetLayout(const VulkanContext& context);
    void createDescriptorSets(const VulkanContext& context, const std::vector<VkBuffer>& uniformBuffers, const VulkanTexture& texture);
    void createDescriptorPool(const VulkanContext& context);
    void createUniformBuffers(const VulkanContext& context);
    void createRenderPass(const VulkanContext& context, const VulkanSwapChainManager& swapChainManager);
    void createGraphicsPipeline(const VulkanContext& context, const VulkanSwapChainManager& swapChain);
    VkShaderModule createShaderModule(const VulkanContext& context, const std::vector<char>& code);
};
