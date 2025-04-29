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