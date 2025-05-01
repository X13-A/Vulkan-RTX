#include "Constants.hpp"
#include "Vulkan_GLFW.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;
const uint32_t GLFW_WINDOW_WIDTH = 1200;
const uint32_t GLFW_WINDOW_HEIGHT = 600;
const char* GLFW_WINDOW_NAME = "Vulkan";

const std::string MODEL_PATH = "models/viking_room/viking_room.obj";
const std::string TEXTURE_PATH = "models/viking_room/viking_room.png";

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const std::vector<const char*> VALIDATION_LAYERS =
{
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
