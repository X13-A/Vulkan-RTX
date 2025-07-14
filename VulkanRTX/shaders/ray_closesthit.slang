#include "ray_common.slang"

// Structure pour les vertex data
struct Vertex
{
    float3 position;
    float2 texCoord;
    float3 normal;
};

// Structure pour l'uniform buffer (binding 2)
struct SceneData
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 viewInverse;
    float4x4 projInverse;
};

// Attributs du triangle (coordonnées barycentriques)
struct Attributes
{
    float2 barycentrics;
};

// Ressources
RaytracingAccelerationStructure scene;              // Binding 0
ConstantBuffer<SceneData> sceneData;                // Binding 2
StructuredBuffer<Vertex> vertexBuffer;              // Binding 3
StructuredBuffer<uint> indexBuffer;                 // Binding 4
Texture2D texture0;                                 // Binding 5
SamplerState sampler0;                              // Binding 5

[shader("closesthit")]
void main(inout RayPayload payload, in Attributes attribs)
{
    // Index du triangle touché
    uint triangleIndex = PrimitiveIndex();

    // Coordonnées barycentriques complètes
    float3 barycentrics = float3(
        1.0 - attribs.barycentrics.x - attribs.barycentrics.y,
        attribs.barycentrics.x,
        attribs.barycentrics.y
    );
    
    // Option 1: Affichage des coordonnées barycentriques (debug simple)
    payload.color = barycentrics;
    return;
}