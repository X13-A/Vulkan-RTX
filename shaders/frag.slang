struct FragmentInput 
{
    float3 fragColor : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
};

// TODO: Use slang reflexion system
[[vk::binding(1)]] Sampler2D myTextureSampler;

void main(FragmentInput input, out float4 outColor : SV_Target0) 
{
    float4 textureColor = myTextureSampler.Sample(input.fragTexCoord);
    outColor = textureColor;
}