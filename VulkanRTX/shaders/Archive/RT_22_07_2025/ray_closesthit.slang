#include "ray_common.slang"

// Structure pour les vertex data
struct Vertex
{
    float3 position;
    float2 texCoord;
    float3 normal;
};

struct Attributes
{
    float2 barycentrics;
};

struct InstanceData
{
    float4x4 normalMatrix;
    uint meshOffset;
    float padding1;
    float padding2;
    float padding3;
};

struct MeshData
{
    uint indexOffset;
    uint vertexOffset;
};

// Ressources
[[vk::binding(0)]] RaytracingAccelerationStructure scene;
[[vk::binding(2)]] ConstantBuffer<SceneData> sceneData;
[[vk::binding(3)]] ByteAddressBuffer vertexBufferRaw;
[[vk::binding(4)]] StructuredBuffer<uint> indexBuffer;
[[vk::binding(5)]] StructuredBuffer<MeshData> meshDataBuffer;
[[vk::binding(6)]] StructuredBuffer<InstanceData> instanceDataBuffer;
[[vk::binding(7)]] Sampler2D texture0[256];

Vertex readVertex(uint vertexIndex)
{
    uint baseOffset = vertexIndex * 32;
    
    Vertex v;
    // Lire position (offset 0, 12 bytes = 3 floats)
    v.position.x = asfloat(vertexBufferRaw.Load(baseOffset + 0));
    v.position.y = asfloat(vertexBufferRaw.Load(baseOffset + 4));
    v.position.z = asfloat(vertexBufferRaw.Load(baseOffset + 8));
    
    // Lire texCoord (offset 12, 8 bytes = 2 floats)  
    v.texCoord.x = asfloat(vertexBufferRaw.Load(baseOffset + 12));
    v.texCoord.y = asfloat(vertexBufferRaw.Load(baseOffset + 16));
    
    // Lire normal (offset 20, 12 bytes = 3 floats)
    v.normal.x = asfloat(vertexBufferRaw.Load(baseOffset + 20));
    v.normal.y = asfloat(vertexBufferRaw.Load(baseOffset + 24));
    v.normal.z = asfloat(vertexBufferRaw.Load(baseOffset + 28));
    
    return v;
}

[shader("closesthit")]
void main(inout RayPayload payload, in Attributes attribs)
{
    InstanceData instanceData = instanceDataBuffer[InstanceIndex()];
    uint meshIndex = instanceData.meshOffset + GeometryIndex();
    MeshData meshData = meshDataBuffer[meshIndex];

    uint i0 = indexBuffer[meshData.indexOffset + PrimitiveIndex() * 3 + 0];
    uint i1 = indexBuffer[meshData.indexOffset + PrimitiveIndex() * 3 + 1];
    uint i2 = indexBuffer[meshData.indexOffset + PrimitiveIndex() * 3 + 2];

    Vertex v0 = readVertex(meshData.vertexOffset + i0);
    Vertex v1 = readVertex(meshData.vertexOffset + i1);
    Vertex v2 = readVertex(meshData.vertexOffset + i2);

    float3 barycentrics = float3(
        1.0 - attribs.barycentrics.x - attribs.barycentrics.y,
        attribs.barycentrics.x,
        attribs.barycentrics.y
    );

    float2 uv = barycentrics.x * v0.texCoord + 
                barycentrics.y * v1.texCoord + 
                barycentrics.z * v2.texCoord;
    
    float3 normal = barycentrics.x * v0.normal + 
                    barycentrics.y * v1.normal + 
                    barycentrics.z * v2.normal;
    
    normal = mul(instanceData.normalMatrix, float4(normal, 0)).xyz;
    normal = normalize(normal);

    float4 textureColor = texture0[meshIndex].SampleLevel(uv, 0);
    
    float NdotL = max(0, dot(normal, -SUN_DIR));
    payload.color = textureColor.rgb * NdotL;
    payload.hitGeometry = true;
    payload.t = RayTCurrent();

    float3 origin = WorldRayOrigin();
    float3 direction = WorldRayDirection();
    payload.pos = origin + payload.t * direction;
}