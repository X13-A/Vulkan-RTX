#include "ObjLoader.hpp"
#include <unordered_map>
#include "VulkanGeometry.hpp"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif

std::string normalizePath(const std::string& baseDir, const std::string& textureName)
{
    if (textureName.empty()) return "";

    // Remove "./"
    std::string cleanTextureName = textureName;
    if (cleanTextureName.substr(0, 2) == "./")
    {
        cleanTextureName = cleanTextureName.substr(2);
    }

    return baseDir + cleanTextureName;
}

ModelInfo ObjLoader::loadObj(const std::string& objPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = objPath.substr(0, objPath.find_last_of("/\\") + 1);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath.c_str(), baseDir.c_str(), true))
    {
        throw std::runtime_error("Failed to load OBJ: " + warn + err);
    }

    ModelInfo model;

    // Load materials
    for (const auto& mat : materials)
    {
        PBRMaterialInfo matInfo{};
        matInfo.name = mat.name;

        matInfo.albedoTexture = normalizePath(baseDir, mat.diffuse_texname);
        matInfo.normalTexture = normalizePath(baseDir, mat.normal_texname);
        matInfo.metallicTexture = normalizePath(baseDir, mat.metallic_texname);
        matInfo.roughnessTexture = normalizePath(baseDir, mat.roughness_texname);
        matInfo.aoTexture = normalizePath(baseDir, mat.ambient_texname);
        matInfo.bumpTexture = normalizePath(baseDir, mat.bump_texname);
        matInfo.displacementTexture = normalizePath(baseDir, mat.displacement_texname);

        matInfo.albedoFactor[0] = mat.diffuse[0];
        matInfo.albedoFactor[1] = mat.diffuse[1];
        matInfo.albedoFactor[2] = mat.diffuse[2];

        matInfo.metallicFactor = mat.metallic;
        matInfo.roughnessFactor = mat.roughness;
        matInfo.aoFactor = mat.ambient[0]; // fallback for AO

        model.materials.push_back(matInfo);
    }

    // Load meshes
    for (const auto& shape : shapes)
    {
        MeshInfo mesh;
        std::unordered_map<VulkanVertex, uint32_t> uniqueVertices;

        for (const auto& index : shape.mesh.indices)
        {
            VulkanVertex vertex{};
            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.normal_index >= 0) 
            {
                vertex.normal = 
                {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (index.texcoord_index >= 0) 
            {
                vertex.texCoord =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }

            mesh.indices.push_back(uniqueVertices[vertex]);
        }

        model.meshes.push_back(mesh);

        // Assign material index
        int materialIndex = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[0];
        model.meshMaterialIndices.push_back(materialIndex);
    }

    return model;
}