#include "ray_common.slang"

struct SceneData
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 viewInverse;
    float4x4 projInverse;
    // Ajoutez d'autres paramètres selon vos besoins
};

// Binding 2: Uniform Buffer
ConstantBuffer<SceneData> sceneData;

[shader("miss")]
void main(inout RayPayload payload)
{
    // Direction du rayon pour créer un dégradé simple
    float3 rayDir = WorldRayDirection();
    
    // Dégradé vertical simple (bleu ciel vers blanc)
    float t = 0.5 * (rayDir.y + 1.0);
    payload.color = lerp(float3(0.5, 0.7, 1.0), float3(1.0, 1.0, 1.0), t);
}