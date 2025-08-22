#include "shaders/lib/common.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;

   output.positionCS = TransformObjectToHClip(input.positionOS);
    output.positionCS.z = output.positionCS.w - 0.00001f;
    output.positionSS = output.positionCS;
    output.normalWS = TransformObjectToWorldNormal(input.normalOS.xyz);
    // output.normalWS = input.normalOS.xyz;

    return output;
}

PSOutput PS_Main(PSInput input) : SV_TARGET
{
    float3 normalWS = normalize(input.normalWS);
    float3 col = SampleSkybox(normalWS).rgb;

    PSOutput output;
    output.Target0 = WriteGBuffer0(col, PIXEL_TYPE_SKYBOX);
    output.Target1 = WriteGBuffer1(normalWS);
    output.Target2 = WriteGBuffer2(input.positionSS.z / input.positionSS.w);

    return output;
}


