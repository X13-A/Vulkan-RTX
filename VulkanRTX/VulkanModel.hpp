#pragma once

#include "Constants.hpp"
#include "VulkanGeometry.hpp"
#include "VulkanTexture.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanExtensionFunctions.hpp"
#include "Vulkan_GLFW.hpp"
#include <vector>
#include "Transform.hpp"

struct VulkanModelUBO
{
    alignas(16) glm::mat4 modelMat;
    alignas(16) glm::mat4 viewMat;
    alignas(16) glm::mat4 projMat;
    alignas(16) glm::mat4 normalMat;
};

class VulkanModel
{
public:
    std::string name;
    std::vector<VulkanVertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VulkanTexture albedoTexture;

    std::vector<VkDescriptorSet> descriptorSets;
    
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    Transform transform;

    // Ray tracing
    VkAccelerationStructureKHR blasHandle;
    VkBuffer blasBuffer;
    VkDeviceMemory blasBufferMemory;
    VkDeviceAddress blasBufferAddress;

public:
    void loadObj(std::string objPath);
    
    void init(std::string objPath, std::string texturePath, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool);
    void createDescriptorSets(const VulkanContext& context, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool);
    void createUniformBuffers(const VulkanContext& context);
    
    void cleanup(VkDevice device);

    // New methods for ray tracing
    void createBLAS(
        const VulkanContext& context,
        VulkanCommandBufferManager& commandBufferManager);
};
