
cbuffer ViewProjection : register(b0)
{
    column_major float4x4 viewProj;
};

cbuffer Model : register(b1)
{
    column_major float4x4 model;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    float3 positions[3] =
    {
        float3(50.0, 100.0, 0.0),
        float3(100.0, 0.0, 0.0),
        float3(0.0, 0.0, 0.0)
    };
    float3 colors[3] =
    {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };

    float4 worldPosition = mul(model, float4(positions[vertexId], 1.0));

    VSOutput output;
    output.position = mul(viewProj, worldPosition);
    output.color = colors[vertexId];
    return output;
}

float4 PSMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
