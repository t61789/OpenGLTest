#include "shaders/lib/common.hlsl"
#include "shaders/lib/batch_rendering.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionCS = TransformObjectToHClipBatch(input.positionOS.xyz, input.id);
    output.positionSS = output.positionCS;

    return output;
}

float4 PS_Main(PSInput input) : SV_TARGET
{
    return float4(1,1,1,1);
}
