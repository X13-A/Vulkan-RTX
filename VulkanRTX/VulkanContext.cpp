#include "VulkanContext.hpp"
#include <set>
#include <iostream>
#include "Constants.hpp"
#include "VulkanSwapChainManager.hpp"
#include <regex>
#include "VulkanExtensionFunctions.hpp"

bool QueueFamilyIndices::isComplete()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

void printColoredValidationMessage(const std::string& message, std::string titleColor, std::string bodyColor)
{
    std::regex pattern(R"(^([^\:]+:))", std::regex::icase);
    std::smatch match;
    std::string formattedMessage = std::regex_replace(message, std::regex(": "), ":\n", std::regex_constants::format_first_only);

    if (std::regex_search(formattedMessage, match, pattern)) {
        std::string highlighted = titleColor + match.str(1) + bodyColor;
        std::string formatted = std::regex_replace(formattedMessage, pattern, highlighted, std::regex_constants::format_first_only);
        std::cerr << std::endl << formatted << std::endl;
    }
    else 
    {
        std::cerr << std::endl << formattedMessage << std::endl;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    // Color output based on severity
    const char* color;
    switch (messageSeverity) 
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: color = "\033[90m"; break; // gray
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    color = "\033[36m"; break; // cyan
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: color = "\033[33m"; break; // yellow
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   color = "\033[31m"; break; // red
        default: color = "\033[0m"; break; // White
    }
    printColoredValidationMessage("[Vulkan] " + std::string(pCallbackData->pMessage), color, "\033[0m");

    return VK_FALSE;
}

VkResult VulkanContext::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanContext::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void VulkanContext::init()
{
    createInstance();
    setupDebugMessenger();
}

void VulkanContext::initDevice()
{
    pickPhysicalDevice();
    createLogicalDevice();
}


void VulkanContext::setupDebugMessenger()
{
    if (!ENABLE_VALIDATION_LAYERS) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = VulkanContext::debugCallback;
    createInfo.pUserData = nullptr;

    if (VulkanContext::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
    std::cout << "Debug messenger successfully setup" << std::endl;
}

bool VulkanContext::checkValidationLayerSupport() const
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : VALIDATION_LAYERS)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> VulkanContext::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLE_VALIDATION_LAYERS)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> vkExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensions.data());

    return extensions;
}

void VulkanContext::createInstance()
{
    if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
    {
        throw std::runtime_error("VK validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = GLFW_WINDOW_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;



    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkValidationFeatureEnableEXT enabledValidationFeatures[] =
    {
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };

    VkValidationFeaturesEXT validationFeatures{};
    validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validationFeatures.enabledValidationFeatureCount = sizeof(enabledValidationFeatures) / sizeof(VkValidationFeatureEnableEXT);
    validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures;

    if (ENABLE_VALIDATION_LAYERS) 
    {
        createInfo.pNext = &validationFeatures;
    }

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;

    if (ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanContext::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // RT validation environment check
    char* envVar = nullptr;
    size_t envVarSize = 0;
    errno_t err = _dupenv_s(&envVar, &envVarSize, "NV_ALLOW_RAYTRACING_VALIDATION");

    bool rtValidationEnabled = false;
    if (err == 0 && envVar != nullptr)
    {
        rtValidationEnabled = (std::string(envVar) == "1");
        free(envVar);
    }

    if (!rtValidationEnabled)
    {
        std::cerr << "Ray tracing validation disabled (set NV_ALLOW_RAYTRACING_VALIDATION=1 to enable)" << std::endl;
    }

    // Build feature chain
    VkPhysicalDeviceRayTracingValidationFeaturesNV validationFeatures{};
    validationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV;
    validationFeatures.pNext = nullptr;

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
    bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bufferDeviceAddressFeatures.pNext = &validationFeatures;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingPipelineFeatures.pNext = &bufferDeviceAddressFeatures;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.features = deviceFeatures;
    deviceFeatures2.pNext = &accelerationStructureFeatures;

    // Query features with the FULL chain
    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

    // Check feature support BEFORE enabling
    if (!accelerationStructureFeatures.accelerationStructure)
    {
        throw std::runtime_error("Acceleration structure feature not supported!");
    }
    if (!rayTracingPipelineFeatures.rayTracingPipeline)
    {
        throw std::runtime_error("Ray tracing pipeline feature not supported!");
    }
    if (!bufferDeviceAddressFeatures.bufferDeviceAddress)
    {
        throw std::runtime_error("Buffer device address feature not supported!");
    }
    if (!validationFeatures.rayTracingValidation)
    {
        std::cerr << "RT validation features are not available." << std::endl;
    }

    // Enable required features AFTER validation
    accelerationStructureFeatures.accelerationStructure = VK_TRUE;
    rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
    bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

    // Only enable RT validation if supported AND environment variable is set
    if (validationFeatures.rayTracingValidation && rtValidationEnabled)
    {
        validationFeatures.rayTracingValidation = VK_TRUE;
        std::cout << "Ray tracing validation enabled!" << std::endl;
    }
    else
    {
        validationFeatures.rayTracingValidation = VK_FALSE;
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &deviceFeatures2;  // Feature chain already built
    createInfo.pEnabledFeatures = nullptr;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();

    if (ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

void VulkanContext::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) 
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::cout << "Available Physical Devices:\n";
    for (const auto& device : devices) 
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        std::cout << "Device: " << deviceProperties.deviceName << "\n";
        std::cout << "  Type: ";
        switch (deviceProperties.deviceType) 
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: std::cout << "Integrated GPU\n"; break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: std::cout << "Discrete GPU\n"; break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: std::cout << "Virtual GPU\n"; break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU: std::cout << "CPU\n"; break;
            default: std::cout << "Other\n";
        }
        std::cout << "  API Version: "
            << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
            << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
            << VK_VERSION_PATCH(deviceProperties.apiVersion) << "\n";

        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    std::cout << "Successfully found suitable GPU" << std::endl;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    std::cout << "SELECTED GPU: " << props.deviceName << " (Type: " << props.deviceType << ")" << std::endl;

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) 
    {
        if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) 
        {
            std::cout << "GPU VRAM: " << memProps.memoryHeaps[i].size / (1024.0f * 1024.0f * 1024.0f) << "GB" << std::endl;
        }
    }
}

bool VulkanContext::isDeviceSuitable(VkPhysicalDevice physicalDevice) const
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = VulkanSwapChainManager::querySwapChainSupport(physicalDevice, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        std::cout << "Required VK extensions are all suported" << std::endl;
    }
    if (swapChainAdequate)
    {
        std::cout << "VK swapchain suitable" << std::endl;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice) const
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(REQUIRED_DEVICE_EXTENSIONS.begin(), REQUIRED_DEVICE_EXTENSIONS.end());

    for (const VkExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    if (!requiredExtensions.empty()) 
    {
        for (const auto& ext : requiredExtensions) 
        {
            std::cerr << "[Missing Extension] " << ext << "\n";
        }
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice physicalDevice) const
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;

            if (!(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) 
            {
                std::cout << " Error: Graphics queue does not support COMPUTE !" << std::endl;
            }
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        std::cout << std::endl;

        if (indices.isComplete())
        {
            break;
        }
        i++;
    }
    return indices;
}

void VulkanContext::loadFunctionPointers()
{
    loadExtensionFunctions(device);
}

void VulkanContext::cleanup()
{
    if (ENABLE_VALIDATION_LAYERS) 
    {
        VulkanContext::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}
