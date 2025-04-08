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

// Vertex shader
VertexOutput main(VertexInput input) 
{
    VertexOutput output;
    output.svPosition = float4(input.position, 1.0);
    output.fragColor = input.color;
    return output;
}