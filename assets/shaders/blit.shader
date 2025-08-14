#include "shaders/lib/common.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionCS = float4(input.positionOS.xy, 0.0f, 1.0f);
    output.uv = input.positionOS.xy * 0.5f + 0.5f;

    return output;
}

Texture2D _MainTex;
SamplerState _MainTexSampler;

float4 PS_Main(PSInput input) : SV_TARGET
{
    float4 albedo = _MainTex.Sample(_MainTexSampler, input.uv);

    return float4(albedo.rgb, 1.0f);
}
