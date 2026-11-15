#include "common.hlsli"

cbuffer ViewProjection : register(b0)
{
    float4x4 viewProj;
};

cbuffer Model : register(b1)
{
    float4x4 model;
};

// One quad per instance. Numbers are in bits for each value
// x(5) | y(9) | z(5) | normalIndex(3) | layer(10)
StructuredBuffer<uint> quads : register(t0);

static const float2 FRONT_CORNERS[6] =
{
    float2(0, 0), float2(1, 0), float2(1, 1),
    float2(1, 1), float2(0, 1), float2(0, 0)
};

static const float2 BACK_CORNERS[6] =
{
    float2(0, 0), float2(1, 1), float2(1, 0),
    float2(1, 1), float2(0, 0), float2(0, 1)
};

VSOutput VSMain(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    uint quad = quads[instanceId];

    float x = float(quad & 31u);
    float y = float((quad >> 5) & 511u);
    float z = float((quad >> 14) & 31u);
    uint normalIndex = (quad >> 19) & 7u;
    uint layer = quad >> 22;

    int perpendicularAxis = int(normalIndex) / 2;
    bool isPositiveFace = (normalIndex % 2) == 0;

    uint index = vertexId % 6;
    float2 offset = isPositiveFace ? FRONT_CORNERS[index] : BACK_CORNERS[index];

    float3 position = float3(x, y, z);
    int uAxis = (perpendicularAxis + 1) % 3;
    int vAxis = (perpendicularAxis + 2) % 3;
    position[uAxis] += offset.x;
    position[vAxis] += offset.y;

    float3 normal = float3(0.0, 0.0, 0.0);
    normal[perpendicularAxis] = isPositiveFace ? 1.0 : -1.0;

    float4 worldPosition = mul(model, float4(position, 1.0));

    // TODO: return fog

    VSOutput output;
    output.position = mul(viewProj, worldPosition);
    output.normal = normal;

    // Water types: regular(6), Sector-R(14), Utopia(17)
    output.isWater = (layer == 6u || layer == 14u || layer == 17u) ? 1.0 : 0.0;

    // TODO: return animation based on time for water

    bool swapUV = (perpendicularAxis == 0 || perpendicularAxis == 1);
    float2 uv = swapUV ? float2(offset.y, offset.x) : offset;
    output.texCoord = float3(uv, float(layer));

    return output;
}
