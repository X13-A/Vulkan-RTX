#pragma once
#include <cstdint>
#include <string>

const int MAX_FRAMES_IN_FLIGHT = 2;
const uint32_t GLFW_WINDOW_WIDTH = 800;
const uint32_t GLFW_WINDOW_HEIGHT = 600;
const char* GLFW_WINDOW_NAME = "Vulkan";

const std::string MODEL_PATH = "models/viking_room/viking_room.obj";
const std::string TEXTURE_PATH = "models/viking_room/viking_room.png";


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
