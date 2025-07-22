#include "ray_common.slang"

struct FragmentInput 
{
    float2 fragTexCoord : TEXCOORD0;
};

// Samplers for G-buffer
[[vk::binding(1)]] Sampler2D depthSampler;
[[vk::binding(2)]] Sampler2D normalSampler;
[[vk::binding(3)]] Sampler2D albedoSampler;

void main(FragmentInput input, out float4 outColor : SV_Target0) 
{
    float2 uv = input.fragTexCoord;
    uv.y = 1 - uv.y;
    float3 encodedNormal = normalSampler.Sample(uv).rgb;
    float3 normal = normalize((encodedNormal - 0.5) * 2.0);
    float4 albedo = albedoSampler.Sample(uv);
    float depth = depthSampler.Sample(uv).r;

    float lightIntensity = max(dot(normal, -SUN_DIR),0) + 0.02;
    outColor = float4(albedo.rgb * lightIntensity, 1);
}
