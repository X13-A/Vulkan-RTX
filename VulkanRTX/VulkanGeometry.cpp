#include "VulkanGeometry.hpp"
#include "Vulkan_GLFW.hpp"
#include "GLM_defines.hpp"

bool VulkanVertex::operator==(const VulkanVertex& other) const
{
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}

VkVertexInputBindingDescription VulkanVertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VulkanVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> VulkanVertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(VulkanVertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(VulkanVertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(VulkanVertex, texCoord);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(VulkanVertex, normal);

    return attributeDescriptions;
}

namespace std
{
    size_t hash<VulkanVertex>::operator()(VulkanVertex const& vertex) const
    {
        size_t h1 = hash<glm::vec3>()(vertex.pos);
        size_t h2 = hash<glm::vec3>()(vertex.color);
        size_t h3 = hash<glm::vec2>()(vertex.texCoord);
        size_t h4 = hash<glm::vec3>()(vertex.normal);

        size_t combined = h1 ^ (h2 << 1);
        combined = (combined >> 1) ^ (h3 << 1);
        combined = (combined >> 1) ^ (h4 << 1);

        return combined;
    }
}
