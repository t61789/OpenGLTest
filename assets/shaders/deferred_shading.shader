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
    float pixelType;
    ReadGBuffer0(input.uv, albedo, pixelType);
    ReadGBuffer1(input.uv, normalWS);

    if (pixelType == PIXEL_TYPE_LIT)
    {
        // return float4(lerp(albedo, normalWS, 0.5f), 1.0f);
        float3 finalColor = Lit(input.positionSS.xyz, normalWS, albedo);
        return float4(finalColor, 1.0f);
    }
    else
    {
        return float4(albedo, 1.0f);
    }
}