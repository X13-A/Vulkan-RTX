struct UniformBufferObject 
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
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
    output.svPosition = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.position, 1.0))));
    output.fragColor = input.color;
    output.fragTexCoord = input.texCoord;
    output.fragNormal = input.normal;
    return output;
}
