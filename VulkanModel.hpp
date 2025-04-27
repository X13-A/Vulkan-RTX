#pragma once

#include "Constants.hpp"
#include <vector>
#include "VulkanContext.hpp"
#include "VulkanGeometry.hpp"
#include "VulkanTexture.hpp"

class VulkanModel
{
public:
    std::vector<VulkanVertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VulkanTexture texture;

public:
    void load(std::string path);
};
