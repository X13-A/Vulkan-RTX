#pragma once

#include "GLM_defines.hpp"
#include "Vulkan_GLFW.hpp"
#include <array>

struct VulkanVertex
{
    glm::vec3 pos = { 0, 0, 0 };
    glm::vec2 texCoord = { 0, 0 };
    glm::vec3 normal = { 0, 0, 0 };
    glm::vec3 tangent = { 0, 0, 0 };
    glm::vec3 bitangent = { 0, 0, 0 };

    bool operator==(const VulkanVertex& other) const;

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
};

namespace std
{
    template<> struct hash<VulkanVertex>
    {
        size_t operator()(VulkanVertex const& vertex) const;
    };
}
