#include "VulkanRayTracingFunctions.hpp"
#include <stdexcept>

PFN_vkCreateAccelerationStructureKHR rt_vkCreateAccelerationStructureKHR = nullptr;
PFN_vkDestroyAccelerationStructureKHR rt_vkDestroyAccelerationStructureKHR = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR rt_vkGetAccelerationStructureBuildSizesKHR = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR rt_vkCmdBuildAccelerationStructuresKHR = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR rt_vkGetAccelerationStructureDeviceAddressKHR = nullptr;

void loadRayTracingFunctions(VkDevice device)
{
    rt_vkCreateAccelerationStructureKHR =
        (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(
            device, "vkCreateAccelerationStructureKHR");

    rt_vkDestroyAccelerationStructureKHR =
        (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(
            device, "vkDestroyAccelerationStructureKHR");

    rt_vkGetAccelerationStructureBuildSizesKHR =
        (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(
            device, "vkGetAccelerationStructureBuildSizesKHR");

    rt_vkCmdBuildAccelerationStructuresKHR =
        (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(
            device, "vkCmdBuildAccelerationStructuresKHR");

    rt_vkGetAccelerationStructureDeviceAddressKHR =
        (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(
            device, "vkGetAccelerationStructureDeviceAddressKHR");

    if (!rt_vkCreateAccelerationStructureKHR ||
        !rt_vkDestroyAccelerationStructureKHR ||
        !rt_vkGetAccelerationStructureBuildSizesKHR ||
        !rt_vkCmdBuildAccelerationStructuresKHR ||
        !rt_vkGetAccelerationStructureDeviceAddressKHR)
    {
        throw std::runtime_error("Échec du chargement des fonctions ray tracing!");
    }
}