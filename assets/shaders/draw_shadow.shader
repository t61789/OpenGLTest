#include "shaders/lib/common.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionCS = TransformObjectToHClip(input.positionOS.xyz);
    output.positionSS = output.positionCS;

    return output;
}

float4 PS_Main(PSInput input) : SV_TARGET
{
    return float4(input.positionSS.z / input.positionSS.w);
}
