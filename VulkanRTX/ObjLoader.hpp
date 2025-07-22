#pragma once

#include <string>
#include <vector>
#include "VulkanGeometry.hpp";

struct PBRMaterialInfo 
{
    std::string name;

    std::string albedoTexture;
    std::string normalTexture;
    std::string metallicTexture;
    std::string roughnessTexture;
    std::string aoTexture;

    float albedoFactor[3] = { 1.0f, 1.0f, 1.0f };
    float metallicFactor = 0.0f;
    float roughnessFactor = 1.0f;
    float aoFactor = 1.0f;

    std::string bumpTexture;
    std::string displacementTexture;
};

struct MeshInfo
{
    std::vector<VulkanVertex> vertices;
    std::vector<uint32_t> indices;
};

struct ModelInfo
{
    std::vector<MeshInfo> meshes;
    std::vector<PBRMaterialInfo> materials;
    std::vector<int> meshMaterialIndices;
};

class ObjLoader
{
public:
    static ModelInfo loadObj(const std::string& objPath);
};