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
    float3 normal = (encodedNormal - 0.5) * 2.0;
    float4 albedo = albedoSampler.Sample(uv);

    float depth = depthSampler.Sample(uv).r;

    if (uv.x < 0.33)
    {
        outColor = float4(depth, 0, 0, 1.0);
    }
    else if (uv.x > 0.33 && uv.x < 0.66)
    {
        outColor = float4(normal, 1.0);
    }
    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float3 lightColor = float3(5, 5, 5);
    float lightIntensity = max(dot(normal, lightDir),0) + 0.02;
    outColor = float4(albedo.rgb * lightColor * lightIntensity, 1);
}
