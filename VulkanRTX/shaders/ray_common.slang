struct RayPayload
{
    float3 color;
    float t;
    bool hitGeometry;
    float3 pos;
};

struct SceneData
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 viewInverse;
    float4x4 projInverse;
    float3 cameraPos;
    float padding;
    float2 nearFar;
    int spp;
};

static float3 SKYBOX_COLOR = float3(1, 1, 1);
static float3 SUN_DIR = normalize(-float3(0.3, 0.5, 0.5));

float3 getSkyLight(float3 rayDir)
{
    float NdotL = max(0, dot(rayDir, -SUN_DIR) - 0.95) * 500;
    return float3(NdotL, NdotL, NdotL);
}

float3 getSkyColor(float3 rayDir)
{
    float t = 0.5 * rayDir.y + 0.5;
    float3 skyColor = lerp(float3(0.6, 0.7, 0.9), float3(0.1, 0.1, 0.2), 1.0 - t);

    float sunDot = dot(rayDir, -SUN_DIR);
    float sunSize = 0.995; // controls sharpness of sun disk
    float sun = smoothstep(sunSize, sunSize + 0.0005, sunDot);
    float3 sunColor = float3(20.0, 18.0, 15.0); // bright warm sun

    return skyColor + sun * sunColor ;
}