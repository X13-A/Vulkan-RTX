// Structure for fragment shader input
struct FragmentInput {
    float3 fragColor : COLOR0;
};

// Fragment shader
void main(FragmentInput input, out float4 outColor : SV_Target0) {
    outColor = float4(input.fragColor, 1.0);
}