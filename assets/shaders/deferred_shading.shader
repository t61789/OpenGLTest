#include "shaders/lib/common.hlsl"
#include "shaders/lib/lighting.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionCS = float4(input.positionOS.xy, 0.0f, 1.0f);
    output.uv = input.positionOS.xy * 0.5f + 0.5f;

    return output;
}

float4 PS_Main(PSInput input) : SV_TARGET
{
    float3 albedo, normalWS;
    float pixelType, sceneDepth;
    ReadGBuffer0(input.uv, albedo, pixelType);
    ReadGBuffer1(input.uv, normalWS);
    ReadGBuffer2(input.uv, sceneDepth);

    float3 positionWS = RebuildWorldPosition(input.uv, sceneDepth);

    float4 finalColor = float4(0,0,0,1);
    if (pixelType == PIXEL_TYPE_LIT)
    {
        float3 col = Lit(positionWS, normalWS, albedo);
        finalColor.rgb = col;
    }
    else
    {
        finalColor.rgb = albedo;
    }

    return finalColor;
}