struct RayPayload
{
    float3 color;
    float t;
    bool hitGeometry;
    float3 pos;
    int depth;
    int maxDepth;
};

struct SceneData
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 viewInverse;
    float4x4 projInverse;
    float3 cameraPos;
    uint maxDepth;
    float2 nearFar;
    int spp;
};

static float3 SUN_DIR = normalize(-float3(0.5, 1, 0.5));
//static float3 SUN_COLOR = float3(15.0, 12.0, 8.0) * 1.0;
static float3 SUN_COLOR = float3(10.0, 10.0, 10.0) * 50.0;
static float3 SKY_COLOR = float3(62.0 / 255.0, 105.0 / 255.0, 196.0 / 255.0) * 1;

float3 getSkyLight(float3 rayDir)
{
    float3 sunDir = -SUN_DIR;
    float sunDot = max(0.0, dot(rayDir, sunDir) - 0.98) * 20.0;
    return SKY_COLOR + SUN_COLOR * sunDot;
}

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

RayPayload traceRay(RaytracingAccelerationStructure scene, float3 origin, float3 rayDir)
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