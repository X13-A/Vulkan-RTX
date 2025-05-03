struct VertexInput
{
    float3 position : POSITION;
    float3 color    : COLOR0;
    float2 texCoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
}

struct VertexOutput 
{
    float2 fragTexCoord : TEXCOORD0;
    float4 position : SV_POSITION;
};

VertexOutput main(VertexInput input) 
{
    VertexOutput output;

    output.position = float4(input.position, 1.0);
    output.fragTexCoord = input.texCoord + input.normal.xy * 0.0000001 + input.color.xy * 0.00000001;

    return output;
}
