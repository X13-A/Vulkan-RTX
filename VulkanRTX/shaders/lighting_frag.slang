struct FragmentInput 
{
    float2 fragTexCoord : TEXCOORD0;
};

// Samplers for G-buffer
[[vk::binding(2)]] Sampler2D normalSampler;
[[vk::binding(1)]] Sampler2D depthSampler;

void main(FragmentInput input, out float4 outColor : SV_Target0) 
{
    float2 uv = input.fragTexCoord;
    uv.y = 1 - uv.y;
    float3 encodedNormal = normalSampler.Sample(uv).rgb;
    float3 normal = (encodedNormal - 0.5) * 2.0;

    float depth = depthSampler.Sample(uv).r;

    if (uv.x < 0.33)
    {
        outColor = float4(depth, 0, 0, 1.0);
    }
    else if (uv.x > 0.33 && uv.x < 0.66)
    {
        outColor = float4(normal, 1.0);
    }
    else
    {
        float3 lightDir = normalize(float3(0.0, 1.0, 0.0));
        float light = dot(normal, lightDir);
        outColor = float4(light, light, light, 1);
    }
}
