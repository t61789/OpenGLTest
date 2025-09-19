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

    float4 finalColor = float4(0,0,0,1);
    if (pixelType == PIXEL_TYPE_LIT)
    {
        float3 col = Lit(input.positionSS.xyz, normalWS, albedo);
        finalColor.rgb = col;
        // finalColor.rgb = lerp(finalColor.rgb, normalWS, 0.99f);
    }
    else
    {
        finalColor.rgb = albedo;
        // finalColor.rgb = normalWS;
    }

    return finalColor;
}