#include "shaders/lib/common.hlsl"
#include "shaders/lib/lighting.hlsl"

cbuffer PerMaterialCBuffer : register(b5)
{
    float4 _Albedo;
};

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionWS = TransformObjectToWorld(input.positionOS.xyz);
    output.positionCS = TransformWorldToHClip(output.positionWS);
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

    float3 col = Lit(input.positionWS, normalWS, albedo);

    return finalColor;
}
