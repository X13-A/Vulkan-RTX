#include "VulkanRayTracingFunctions.hpp"
#include <stdexcept>
#include <unordered_map>

PFN_vkCreateAccelerationStructureKHR rt_vkCreateAccelerationStructureKHR = nullptr;
PFN_vkDestroyAccelerationStructureKHR rt_vkDestroyAccelerationStructureKHR = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR rt_vkGetAccelerationStructureBuildSizesKHR = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR rt_vkCmdBuildAccelerationStructuresKHR = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR rt_vkGetAccelerationStructureDeviceAddressKHR = nullptr;

// Additional functions
PFN_vkGetBufferDeviceAddressKHR rt_vkGetBufferDeviceAddressKHR = nullptr;
PFN_vkCreateRayTracingPipelinesKHR rt_vkCreateRayTracingPipelinesKHR = nullptr;
PFN_vkGetRayTracingShaderGroupHandlesKHR rt_vkGetRayTracingShaderGroupHandlesKHR = nullptr;
PFN_vkCmdTraceRaysKHR rt_vkCmdTraceRaysKHR = nullptr;

void loadRayTracingFunctions(VkDevice device)
{
    std::unordered_map<std::string, PFN_vkVoidFunction*> functionMap = 
    {
        {"vkCreateAccelerationStructureKHR", (PFN_vkVoidFunction*)&rt_vkCreateAccelerationStructureKHR},
        {"vkDestroyAccelerationStructureKHR", (PFN_vkVoidFunction*)&rt_vkDestroyAccelerationStructureKHR},
        {"vkGetAccelerationStructureBuildSizesKHR", (PFN_vkVoidFunction*)&rt_vkGetAccelerationStructureBuildSizesKHR},
        {"vkCmdBuildAccelerationStructuresKHR", (PFN_vkVoidFunction*)&rt_vkCmdBuildAccelerationStructuresKHR},
        {"vkGetAccelerationStructureDeviceAddressKHR", (PFN_vkVoidFunction*)&rt_vkGetAccelerationStructureDeviceAddressKHR},
        {"vkGetBufferDeviceAddressKHR", (PFN_vkVoidFunction*)&rt_vkGetBufferDeviceAddressKHR},
        {"vkCreateRayTracingPipelinesKHR", (PFN_vkVoidFunction*)&rt_vkCreateRayTracingPipelinesKHR},
        {"vkGetRayTracingShaderGroupHandlesKHR", (PFN_vkVoidFunction*)&rt_vkGetRayTracingShaderGroupHandlesKHR},
        {"vkCmdTraceRaysKHR", (PFN_vkVoidFunction*)&rt_vkCmdTraceRaysKHR}
    };

    // Load all functions
    for (const auto& [functionName, functionPtr] : functionMap) 
    {
        *functionPtr = vkGetDeviceProcAddr(device, functionName.c_str());

        if (*functionPtr == nullptr) 
        {
            std::string errorMsg = functionName + " not found!";
            throw std::runtime_error(errorMsg);
        }
    }
}