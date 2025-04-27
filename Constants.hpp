#pragma once

#include <cstdint>
#include <string>
#include <vector>

extern const int MAX_FRAMES_IN_FLIGHT;
extern const uint32_t GLFW_WINDOW_WIDTH;
extern const uint32_t GLFW_WINDOW_HEIGHT;
extern const char* GLFW_WINDOW_NAME;

extern const std::string MODEL_PATH;
extern const std::string TEXTURE_PATH;
extern const bool ENABLE_VALIDATION_LAYERS;
extern const std::vector<const char*> VALIDATION_LAYERS;
extern const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS;

//#define VK_USE_PLATFORM_WIN32_KHR
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/hash.hpp>
//
//#define GLFW_INCLUDE_VULKAN
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>
