struct FragmentInput 
{
    float3 fragColor : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
    float3 fragNormal : TEXCOORD1;
};

[[vk::binding(0)]] Sampler2D albedoSampler;  // Albedo texture for sampling

void main(FragmentInput input, out float4 outNormal : SV_Target0)
{
    float4 textureColor = albedoSampler.Sample(input.fragTexCoord);
    outNormal = float4(abs(input.fragNormal), 1); // Normal output for deferred rendering (in [0, 1] range)
}
