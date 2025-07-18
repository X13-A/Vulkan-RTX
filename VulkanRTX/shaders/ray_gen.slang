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

[[vk::binding(7)]] Sampler2D depthBuffer;
[[vk::binding(8)]] Sampler2D normalBuffer;
[[vk::binding(9)]] Sampler2D albedoBuffer;

float3 random_unit_vector(inout uint seed) 
{
    while (true) 
    {
        seed = seed * 1664525u + 1013904223u;
        float x = (float(seed & 0x00FFFFFFu) / float(0x00FFFFFFu)) * 2.0 - 1.0;
        
        seed = seed * 1664525u + 1013904223u;
        float y = (float(seed & 0x00FFFFFFu) / float(0x00FFFFFFu)) * 2.0 - 1.0;
        
        seed = seed * 1664525u + 1013904223u;
        float z = (float(seed & 0x00FFFFFFu) / float(0x00FFFFFFu)) * 2.0 - 1.0;
        
        float3 p = float3(x, y, z);
        float lensq = dot(p, p);
        
        if (lensq > 1e-10 && lensq <= 1.0) 
        {
            return p / sqrt(lensq);
        }
    }
}

float3 cosine_weighted_hemisphere(const float3 normal, inout uint seed) 
{
    // Generate two random numbers
    seed = seed * 1664525u + 1013904223u;
    float r1 = float(seed & 0x00FFFFFFu) / float(0x00FFFFFFu);
    
    seed = seed * 1664525u + 1013904223u;
    float r2 = float(seed & 0x00FFFFFFu) / float(0x00FFFFFFu);
    
    // Cosine-weighted sampling
    float cos_theta = sqrt(r1);  
    float sin_theta = sqrt(1.0 - r1);  // Polar angle must be biased towards normal
    float phi = 2.0 * 3.14159265359 * r2; // Azimuth must be uniform
    
    float3 direction = float3(
        cos(phi) * sin_theta,
        sin(phi) * sin_theta,
        cos_theta
    );
    
    // Transform to world space
    float3 up = abs(normal.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);
    
    return direction.x * tangent + direction.y * bitangent + direction.z * normal;
}

float3 random_on_hemisphere(const float3 normal, inout uint seed) 
{
    float3 on_unit_sphere = random_unit_vector(seed);
    if (dot(on_unit_sphere, normal) > 0.0) 
    {
        return on_unit_sphere;
    } 
    else 
    {
        return -on_unit_sphere;
    }
}

uint whiteNoise(float2 uv, uint frameCount) 
{
    uint2 pixelPos = uint2(uv * 1024.0);
    uint seed = pixelPos.x + pixelPos.y * 1024 + frameCount * 1048576;
    
    // Simple hash
    seed = seed ^ (seed >> 16);
    seed *= 0x9e3779b9;
    seed = seed ^ (seed >> 16);
    
    return seed;
}

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float3 dirToLight(float3 worldPos)
{
    return normalize(float3(0, 4, 0) - worldPos);
}

RayPayload traceRay (float3 origin, float3 rayDir)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.01;
    ray.TMax = 1000.0;

    RayPayload rayPayload;
    rayPayload.color = float3(0, 0, 0);
    rayPayload.hitGeometry = false;
    rayPayload.t = 0;
    rayPayload.pos = origin;

    TraceRay(scene, 0, 0xFF, 0, 1, 0, ray, rayPayload);
    return rayPayload;
}

[shader("raygeneration")]
void main()
{
    uint2 pixelCoord = DispatchRaysIndex().xy;
    uint2 screenSize = DispatchRaysDimensions().xy;

    float2 uv = (float2(pixelCoord) + 0.5) / float2(screenSize);
    if (uv.x < 0.05 && uv.y < 0.05)
    {
        renderTarget[pixelCoord] = float4(1, 0, 0, 1);
        return;
    }

    // Read GBuffer
    float3 encodedNormal = normalBuffer.SampleLevel(uv, 0).rgb;
    float3 normal = normalize((encodedNormal - 0.5) * 2.0);
    float4 albedo = albedoBuffer.SampleLevel(uv, 0).rgba;
    float depth = depthBuffer.SampleLevel(uv, 0).r;

    // Linearize depth
    float near = sceneData.nearFar.x;
    float far = sceneData.nearFar.y;
    depth = linearize_depth(depth, near, far);

    float2 ndc = uv * 2.0 - 1.0;

    // Compute ray dir
    float4 origin = mul(sceneData.viewInverse, float4(0, 0, 0, 1));
    float4 target = mul(sceneData.projInverse, float4(ndc, 1, 1));
    float3 dir = mul(sceneData.viewInverse, float4(target.xyz, 0)).xyz;
    float3 normalizedDir = normalize(dir);
    
    float3 worldPos = origin.xyz + dir * depth;

    if (depth >= far - 0.1)
    {
        float3 skyColor = getSkyColor(normalizedDir);
        renderTarget[pixelCoord] = float4(skyColor, 1.0);
        return;
    }

    // Force emission on white pixels
    float3 emission = 0;

    uint baseSeed = whiteNoise(uv, pushConstants.frameCount);

    float spp = sceneData.spp;
    float3 giColor = float3(0, 0, 0);
    
    bool useCosine = (spp % 10 == 0);
    if (useCosine)
    {
        spp -= 1;
    }

    for (int i = 0; i < spp; i++)
    {
        uint sampleSeed = baseSeed + i * 1024;
        float3 giRayDir;
        if (!useCosine)
        {
           giRayDir = random_on_hemisphere(normal, sampleSeed);
        }
        else
        {
           giRayDir = cosine_weighted_hemisphere(normal, sampleSeed);
        }

        RayPayload giRayPayload = traceRay(worldPos, giRayDir);
        
        // Handle skybox
        if (!giRayPayload.hitGeometry)
        {
            giColor += giRayPayload.color;
            continue;
        }
        
        // Handle geometry
        RayPayload giShadowPayload = traceRay(giRayPayload.pos, -SUN_DIR);

        // Shadowed secondary intersections don't contribute
        if (!giShadowPayload.hitGeometry)
        {
            if (!useCosine)
            {
                float cos_theta = max(0, dot(giRayDir, normal));
                giColor += giShadowPayload.color * giRayPayload.color * cos_theta;
            }
            else
            {
                giColor += giShadowPayload.color * giRayPayload.color;
            }
        }
    }

    if (!useCosine)
    {
        giColor = emission + albedo.rbg * giColor * (2.0f / spp);
    }
    else
    {
        giColor = emission + albedo.rgb * giColor * (1.0f / spp);
    }
    
    float3 finalColor = giColor;

    renderTarget[pixelCoord] = float4(finalColor, 1.0);
}