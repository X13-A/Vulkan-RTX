struct UniformBufferObject 
{
    float4x4 modelMat;
    float4x4 viewMat;
    float4x4 projMat;
    float4x4 normalMat;
};

ConstantBuffer<UniformBufferObject> ubo;

struct VertexInput
{
    float3 position : POSITION;
    float3 color    : COLOR0;
    float2 texCoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
}

struct VertexOutput 
{
    float4 svPosition : SV_Position;
    float3 fragColor  : COLOR0;
    float2 fragTexCoord : TEXCOORD0;
    float3 fragNormal : TEXCOORD1;
};

VertexOutput main(VertexInput input) 
{
    VertexOutput output;
    output.svPosition = mul(ubo.projMat, mul(ubo.viewMat, mul(ubo.modelMat, float4(input.position, 1.0))));
    output.fragColor = input.color;
    output.fragTexCoord = input.texCoord;
    output.fragNormal = mul(ubo.normalMat, float4(input.normal, 0)).xyz;

    output.fragNormal = (output.fragNormal * 0.5) + 0.5;
    return output;
}
