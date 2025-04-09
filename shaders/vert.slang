struct UniformBufferObject 
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

// Problem here ???
// TODO: How to bind constant buffer in slang with vulkan
ConstantBuffer<UniformBufferObject> ubo;

struct VertexInput
{
    float3 position : POSITION;
    float3 color    : COLOR0;
};

struct VertexOutput 
{
    float4 svPosition : SV_Position;
    float3 fragColor  : COLOR0;
};

VertexOutput main(VertexInput input) 
{
    VertexOutput output;
    output.svPosition = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.position, 1.0))));
    output.fragColor = input.color;
    return output;
}
