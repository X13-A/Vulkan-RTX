#include "VulkanExtensionFunctions.hpp"
#include <stdexcept>
#include <unordered_map>
#include <iostream>

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

PFN_vkSetDebugUtilsObjectNameEXT debug_vkSetDebugUtilsObjectNameEXT = nullptr;
PFN_vkCmdBeginDebugUtilsLabelEXT debug_vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT debug_vkCmdEndDebugUtilsLabelEXT = nullptr;

struct VulkanExtensionFunction
{
    const char* name;
    const bool mandatory;
    PFN_vkVoidFunction* ptr;
};

void loadExtensionFunctions(VkDevice device)
{
    VulkanExtensionFunction functions [] =
    {
        {"vkCreateAccelerationStructureKHR", true, (PFN_vkVoidFunction*)&rt_vkCreateAccelerationStructureKHR},
        {"vkDestroyAccelerationStructureKHR", true, (PFN_vkVoidFunction*)&rt_vkDestroyAccelerationStructureKHR},
        {"vkGetAccelerationStructureBuildSizesKHR", true, (PFN_vkVoidFunction*)&rt_vkGetAccelerationStructureBuildSizesKHR},
        {"vkCmdBuildAccelerationStructuresKHR", true, (PFN_vkVoidFunction*)&rt_vkCmdBuildAccelerationStructuresKHR},
        {"vkGetAccelerationStructureDeviceAddressKHR", true, (PFN_vkVoidFunction*)&rt_vkGetAccelerationStructureDeviceAddressKHR},
        {"vkGetBufferDeviceAddressKHR", true, (PFN_vkVoidFunction*)&rt_vkGetBufferDeviceAddressKHR},
        {"vkCreateRayTracingPipelinesKHR", true, (PFN_vkVoidFunction*)&rt_vkCreateRayTracingPipelinesKHR},
        {"vkGetRayTracingShaderGroupHandlesKHR", true, (PFN_vkVoidFunction*)&rt_vkGetRayTracingShaderGroupHandlesKHR},
        {"vkCmdTraceRaysKHR", true, (PFN_vkVoidFunction*)&rt_vkCmdTraceRaysKHR},
        {"vkSetDebugUtilsObjectNameEXT", false, (PFN_vkVoidFunction*)&debug_vkSetDebugUtilsObjectNameEXT},
        {"vkCmdBeginDebugUtilsLabelEXT", false, (PFN_vkVoidFunction*)&debug_vkCmdBeginDebugUtilsLabelEXT},
        {"vkCmdEndDebugUtilsLabelEXT", false, (PFN_vkVoidFunction*)&debug_vkCmdEndDebugUtilsLabelEXT}
    };

    // Load all functions
    for (const VulkanExtensionFunction& function : functions) 
    {
        *function.ptr = vkGetDeviceProcAddr(device, function.name);

        if (*function.ptr == nullptr)
        {
            std::string errorMsg = std::string(function.name) + " not found!";
            if (function.mandatory)
            {
                throw std::runtime_error(errorMsg);
            }
            else
            {
                std::cerr << errorMsg << std::endl;
            }
        }
    }
}