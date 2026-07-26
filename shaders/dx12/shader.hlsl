
struct VSOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    float2 positions[3] =
    {
        float2(0.0, 0.5),
        float2(0.5, -0.5),
        float2(-0.5, -0.5)
    };
    float3 colors[3] =
    {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };
    
    VSOutput output;
    output.position = float4(positions[vertexId], 0.0, 1.0);
    output.color = colors[vertexId];
    return output;
}

float4 PSMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
