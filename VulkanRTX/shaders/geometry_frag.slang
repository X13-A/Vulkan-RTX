struct FragmentInput 
{
    float2 fragTexCoord : TEXCOORD0;
    float3 fragNormal : TEXCOORD1;
    float3x3 TBN : TEXCOORD2;
};

struct UniformBufferObject 
{
    float4x4 modelMat;
    float4x4 viewMat;
    float4x4 projMat;
    float4x4 normalMat;
    bool debug;
};

[[vk::binding(0, 0)]] 
ConstantBuffer<UniformBufferObject> ubo;

[[vk::binding(0, 1)]] Sampler2D albedoSampler;
[[vk::binding(1, 1)]] Sampler2D bumpSampler;

void main(FragmentInput input, out float4 outNormal : SV_Target0, out float4 outAlbedo : SV_Target2)
{
    outAlbedo = albedoSampler.Sample(input.fragTexCoord);
    
    float3 defaultBump = float3(128.0/255.0, 128.0/255.0, 255.0/255.0);

    // Compute normal using bump map
    float3 bumpMapValue = bumpSampler.Sample(input.fragTexCoord).rgb;
    if (ubo.debug)
    {
        bumpMapValue = defaultBump;
    }
    
    float3 worldNormal = normalize(bumpMapValue * 2.0 - 1.0);
    worldNormal = normalize(mul(worldNormal, input.TBN));

    // Encode normal
    float3 encodedNormal = (worldNormal * 0.5) + 0.5;
    outNormal = float4(encodedNormal, 1);
}