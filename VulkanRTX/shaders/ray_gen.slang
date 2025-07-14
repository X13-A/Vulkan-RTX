#include "ray_common.slang"

struct CameraData
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 viewInverse;
    float4x4 projInverse;
};

RaytracingAccelerationStructure scene;
RWTexture2D<float4> renderTarget;
ConstantBuffer<CameraData> camera;

[shader("raygeneration")]
void main()
{

    // Coordonnées du pixel
    uint2 pixelCoord = DispatchRaysIndex().xy;
    uint2 screenSize = DispatchRaysDimensions().xy;

    // Conversion en coordonnées UV [-1, 1]
    float2 uv = (float2(pixelCoord) + 0.5) / float2(screenSize);
    float2 ndc = uv * 2.0 - 1.0;

    // Calcul du rayon
    float4 origin = mul(camera.viewInverse, float4(0, 0, 0, 1));
    float4 target = mul(camera.projInverse, float4(ndc, 1, 1));
    float3 direction = normalize(mul(camera.viewInverse, float4(target.xyz, 0)).xyz);
    
    // Configuration du rayon
    RayDesc ray;
    ray.Origin = origin.xyz;
    ray.Direction = direction;
    ray.TMin = 0.01;
    ray.TMax = 1000.0;
    
    // Payload
    RayPayload payload;
    payload.color = float3(0, 0, 0);
    
    // Lancement du rayon
    TraceRay(scene, 0, 0xFF, 0, 1, 0, ray, payload);
    
    // Écriture du résultat
    renderTarget[pixelCoord] = float4(payload.color, 1.0);
}