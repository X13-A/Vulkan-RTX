#pragma once

#include "GLM_defines.hpp"
#include "Vulkan_GLFW.hpp"
#include <array>

struct VulkanVertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const VulkanVertex& other) const;

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

namespace std
{
    template<> struct hash<VulkanVertex>
    {
        size_t operator()(VulkanVertex const& vertex) const;
    };
}
