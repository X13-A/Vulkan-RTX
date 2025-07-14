#pragma once;

#include <vulkan/vulkan.h>
#include <vector>
#include "GLM_defines.hpp"
#include "VulkanExtensionFunctions.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanModel.hpp"

struct BLASInstance 
{
    VkAccelerationStructureKHR blas;
    glm::mat4 transform;

    uint32_t instanceId;
    uint32_t hitGroupIndex;
};

class VulkanTLAS 
{
private:
    VkAccelerationStructureKHR tlas;
    VkBuffer tlasBuffer;
    VkDeviceMemory tlasMemory;
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchMemory;

public:
    void createTLAS(const VulkanContext& context, const std::vector<VulkanModel>& models, VulkanCommandBufferManager& commandBufferManager);

    void cleanup(const VulkanContext& context);

    VkAccelerationStructureKHR getTLAS() const;
private:
    void createInstanceBuffer(const VulkanContext& context, const std::vector<BLASInstance>& instances);
    VkAccelerationStructureBuildSizesInfoKHR getBuildSizes(const VulkanContext& context, uint32_t instanceCount);

    void createAccelerationStructure(const VulkanContext& context, VkDeviceSize size);

    void buildTLAS(const VulkanContext& context, uint32_t instanceCount, VkCommandBuffer commandBuffer);

    uint32_t findMemoryType(const VulkanContext& context, uint32_t typeFilter, VkMemoryPropertyFlags properties);
};