#include "shaders/lib/common.hlsl"

cbuffer PerMaterialCBuffer : register(b5)
{
    float4 _Albedo;
};

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionCS = TransformObjectToHClip(input.positionOS.xyz);
    output.normalWS = TransformObjectToWorldNormal(input.normalOS.xyz);
    output.uv = input.uv0;

    return output;
}

TEXTURE2D(_MainTex)

float4 PS_Main(PSInput input) : SV_TARGET
{
    float4 albedo = _MainTex.Sample(_MainTexSampler, input.uv);
    float3 normalWS = normalize(input.normalWS);

    float4 finalColor = albedo * _Albedo;
    // finalColor.rgb = lerp(finalColor.rgb, normalWS * 0.5f + 0.5f, 0.5f);

    return finalColor;
}
