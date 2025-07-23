#include "ray_common.slang"

struct PushConstants
{
    uint frameCount;
};

[[push_constant]]
PushConstants pushConstants;

RaytracingAccelerationStructure scene;
RWTexture2D<float4> renderTarget;
ConstantBuffer<SceneData> sceneData;

[[vk::binding(8)]] Sampler2D depthBuffer;
[[vk::binding(9)]] Sampler2D normalBuffer;
[[vk::binding(10)]] Sampler2D albedoBuffer;
[[vk::binding(11)]] Sampler2D previousFrame;


[shader("raygeneration")]
void main()
{
    uint2 pixelCoord = DispatchRaysIndex().xy;
    uint2 screenSize = DispatchRaysDimensions().xy;

    float2 uv = (float2(pixelCoord) + 0.5) / float2(screenSize);

    // Read GBuffer
    float3 encodedNormal = normalBuffer.SampleLevel(uv, 0).rgb;

    float3 normal = normalize((encodedNormal - 0.5) * 2.0);
    float4 albedo = albedoBuffer.SampleLevel(uv, 0).rgba;
    float depth = depthBuffer.SampleLevel(uv, 0).r;

    // Linearize depth
    float near = sceneData.nearFar.x;
    float far = sceneData.nearFar.y;
    depth = linearize_depth(depth, near, far);

    // Compute ray dir
    float2 ndc = uv * 2.0 - 1.0;
    float4 origin = mul(sceneData.viewInverse, float4(0, 0, 0, 1));
    float4 target = mul(sceneData.projInverse, float4(ndc, 1, 1));
    float3 dir = mul(sceneData.viewInverse, float4(target.xyz, 0)).xyz;
    float3 normalizedDir = normalize(dir);
    
    // World pos
    float3 worldPos = origin.xyz + dir * depth;

    float3 previousColor = previousFrame.SampleLevel(uv, 0).rgb;

    // Sky color
    if (depth >= far - 0.1)
    {
        float3 skyColor = getSkyLight(normalizedDir);
        
        // Blend with previous frame
        float weight = 1.0 / float(pushConstants.frameCount + 1);
        float3 finalColor = lerp(previousColor, skyColor, weight);
        renderTarget[pixelCoord] = float4(finalColor, 1.0);
        return;
    }

    uint baseSeed = whiteNoise(uv, pushConstants.frameCount);
    float spp = sceneData.spp;
    float3 giColor = float3(0, 0, 0);
    
    for (int i = 0; i < spp; i++)
    {
        uint sampleSeed = baseSeed + i * 1024;
        float3 giRayDir = cosine_weighted_hemisphere(normal, sampleSeed);
    
        RayPayload giRayPayload;
        giRayPayload.depth = 0;
        giRayPayload.maxDepth = sceneData.maxDepth;
        giRayPayload.color = float3(0, 0, 0);
        giRayPayload.hitGeometry = false;
        giRayPayload.t = 0;
        giRayPayload.pos = worldPos;

        RayDesc ray;
        ray.Origin = worldPos;
        ray.Direction = giRayDir;
        ray.TMin = 0.01;
        ray.TMax = 1000.0;

        TraceRay(scene, 0, 0xFF, 0, 1, 0, ray, giRayPayload);
    
        giColor += giRayPayload.color;
    }

    float3 currentColor = albedo.rgb * giColor * (1.0f / spp);
    
    // Temporal accumulation
    float weight = 1.0 / float(pushConstants.frameCount + 1);
    float3 finalColor = lerp(previousColor, currentColor, weight);
    
    renderTarget[pixelCoord] = float4(finalColor, 1.0);
}