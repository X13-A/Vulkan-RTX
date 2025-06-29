#include <vulkan/vulkan.h>
#include <vector>
#include "GLM_defines.hpp"
#include "VulkanRayTracingFunctions.hpp"
#include "VulkanContext.hpp"

struct BLASInstance 
{
    VkAccelerationStructureKHR blas;
    glm::mat4 transform;

    uint32_t instanceId;
    uint32_t hitGroupIndex;
};

class TLASBuilder 
{
private:
    const VulkanContext& context;

    // TLAS resources
    VkAccelerationStructureKHR tlas;
    VkBuffer tlasBuffer;
    VkDeviceMemory tlasMemory;
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchMemory;

public:
    TLASBuilder(const VulkanContext& context) : context(context) {}

    VkAccelerationStructureKHR createTLAS(const std::vector<BLASInstance>& instances, VkCommandBuffer commandBuffer);

    void cleanup();

    static void destroyTLAS(const VulkanContext& context, VkAccelerationStructureKHR& tlas);

private:
    void createInstanceBuffer(const std::vector<BLASInstance>& instances);
    VkAccelerationStructureBuildSizesInfoKHR getBuildSizes(uint32_t instanceCount);

    void createAccelerationStructure(VkDeviceSize size);

    void buildTLAS(uint32_t instanceCount, VkCommandBuffer commandBuffer);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};