#pragma once

#include "Constants.hpp"
#include "VulkanGeometry.hpp"
#include "VulkanTexture.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanGraphicsPipeline.hpp"

#include <vector>

class VulkanModel
{
public:
    std::vector<VulkanVertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VulkanTexture texture;

    std::vector<VkDescriptorSet> descriptorSets;
    
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    glm::mat4x4 modelMatrix;

public:
    void load(std::string path, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager);
    void init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanGraphicsPipeline& graphicsPipeline);
    void createDescriptorSets(const VulkanContext& context, const VulkanGraphicsPipeline& graphicsPipeline);
    void createUniformBuffers(const VulkanContext& context);
    void cleanup(VkDevice device);
};
