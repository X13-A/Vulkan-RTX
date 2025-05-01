struct FragmentInput 
{
    float3 fragColor : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
    float3 fragNormal : TEXCOORD1;
};

// TODO: Use slang reflexion system
[[vk::binding(1)]] Sampler2D myTextureSampler;

void main(FragmentInput input, out float4 outColor : SV_Target0) 
{
    float4 textureColor = myTextureSampler.Sample(input.fragTexCoord);
    outColor = float4(abs(input.fragNormal), 1);
}