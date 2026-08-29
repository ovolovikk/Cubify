#ifndef CUBIFY_COMMON_HLSLI
#define CUBIFY_COMMON_HLSLI

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD;
    nointerpolation float3 normal : NORMAL;
    nointerpolation float isWater : WATER;
};

#endif
