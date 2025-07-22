struct FragmentInput 
{
    float2 fragTexCoord : TEXCOORD0;
    float3 fragNormal : TEXCOORD1;
};

[[vk::binding(0, 1)]] Sampler2D albedoSampler;

void main(FragmentInput input, out float4 outNormal : SV_Target0, out float4 outAlbedo : SV_Target2)
{
    outAlbedo = albedoSampler.Sample(input.fragTexCoord);
    outNormal = float4(abs(input.fragNormal), 1); // Encoded normal
}