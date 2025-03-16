// Define vertex positions
static const vector<float, 2> positions[3] = {
    vector<float, 2>(0.0, -0.5),
    vector<float, 2>(0.5, 0.5),
    vector<float, 2>(-0.5, 0.5)
};

// Define vertex colors
static const vector<float, 3> colors[3] = {
    vector<float, 3>(1.0, 0.0, 0.0),
    vector<float, 3>(0.0, 1.0, 0.0),
    vector<float, 3>(0.0, 0.0, 1.0)
};

// Structure for vertex shader output
struct VertexOutput {
    float4 svPosition : SV_Position;
    float3 fragColor  : COLOR0;
};

// Vertex shader
VertexOutput main(uint vertexID : SV_VertexID) 
{
    VertexOutput output;
    output.svPosition = float4(positions[vertexID], 0.0, 1.0);
    output.fragColor = colors[vertexID];
    return output;
}