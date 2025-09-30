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
#include "VulkanMesh.hpp"

struct VulkanModelUBO
{
    alignas(16) glm::mat4 modelMat;
    alignas(16) glm::mat4 viewMat;
    alignas(16) glm::mat4 projMat;
    alignas(16) glm::mat4 normalMat;
    bool debug;
};

struct ShadedMesh
{
    // 1 to 1 relationship
    VulkanMesh mesh;
    VulkanMaterial material;
};

class VulkanModel
{
public:
    std::string name;
    Transform transform;

    std::vector<ShadedMesh> shadedMeshes;

    // Model uniforms
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    // These descriptor sets are used for model-unique data only (no textures)
    std::vector<VkDescriptorSet> modelDescriptorSets;

    // Ray tracing
    VkAccelerationStructureKHR blasHandle;
    VkBuffer blasBuffer;
    VkDeviceMemory blasBufferMemory;
    VkDeviceAddress blasBufferAddress;

public:
    
    void load(ModelInfo info, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorPool descriptorPool);
    void cleanup(VkDevice device);

    void createDescriptorSets(const VulkanContext& context, VkDescriptorPool descriptorPool);
    void createUniformBuffers(const VulkanContext& context);

    void createBLAS(
        const VulkanContext& context,
        VulkanCommandBufferManager& commandBufferManager);

};

// One model has multiple meshes
// One mesh has one material
// One material has a few textures (albedo, etc)

// Eeach model has descriptor sets for their geometry (Transform data)
// Each mesh has specific descriptor sets for it's textures etc

// Each Model builds one BLAS with geometry indexing :
//    - buildInfo.geometryCount = submeshes.size();
//    - buildInfo.pGeometries = geometries.data();

// This allows material indexing in the RT shader