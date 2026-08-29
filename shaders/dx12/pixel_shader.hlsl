#include "common.hlsli"

Texture2DArray<float4> textures : register(t1);
SamplerState blockSampler : register(s0);

static const float3 SUN_DIRECTION = normalize(float3(0.5, 0.8, 0.4));
static const float AMBIENT_INTENSITY = 0.3;
static const float SUN_INTENSITY = 0.6;

float4 PSMain(VSOutput input) : SV_Target
{
    float4 texColor = textures.Sample(blockSampler, input.texCoord);
    if (texColor.a < 0.01)
    {
        discard;
    }

    // Remap to [0, 1] so each side is visibly lit
    float wrapDiffuse = (dot(input.normal, SUN_DIRECTION) + 1.0) * 0.5;

    float faceBrightness;
    if (abs(input.normal.y) > 0.5)
    {
        faceBrightness = input.normal.y > 0.0 ? 1.0 : 0.5; // Top 1.0, bottom 0.5
    }
    else if (abs(input.normal.x) > 0.5)
    {
        faceBrightness = 0.8;
    }
    else
    {
        faceBrightness = 0.7;
    }

    float lighting = wrapDiffuse * SUN_INTENSITY * faceBrightness + AMBIENT_INTENSITY;
    float3 litColor = lighting * texColor.rgb;
    float alpha = input.isWater > 0.5 ? 0.6 : texColor.a;
    return float4(litColor, alpha);
}
