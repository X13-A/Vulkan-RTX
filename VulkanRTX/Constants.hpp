#pragma once

#include <cstdint>
#include <string>
#include <vector>

extern const int MAX_FRAMES_IN_FLIGHT;
extern const uint32_t GLFW_WINDOW_WIDTH;
extern const uint32_t GLFW_WINDOW_HEIGHT;
extern const char* GLFW_WINDOW_NAME;

extern const bool ENABLE_VALIDATION_LAYERS;
extern const std::vector<const char*> VALIDATION_LAYERS;
extern const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS;

// Ray tracing
extern const int RT_MAX_RECURSION_DEPTH;
extern const int RT_RAYGEN_SHADER_INDEX;
extern const int RT_MISS_SHADER_INDEX;
extern const int RT_CLOSEST_HIT_GENERAL_SHADER_INDEX;

extern const int MAX_ALBEDO_TEXTURES;
extern const int FULLSCREEN_QUAD_COUNT;