#include "shaders/lib/common.hlsl"

float4x4 GetIdentity()
{
    return float4x4(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
}

PSInput VS_Main(VSInput input)
{
    float scale[] = {0.1f, 2.7f};
    
    PSInput output;

    output.positionCS = mul(mul(float4(input.positionOS.xyz + float3(input.id, 0, 0), 1.0f), GetIdentity()), _VP);
    // output.positionCS = TransformObjectToHClip(input.positionOS);
    output.positionSS = output.positionCS;
    output.uv = float2(input.id, 0);
    // output.normalWS = TransformObjectToWorldNormal(input.normalOS.xyz);
    // output.normalWS = input.normalOS.xyz;

    return output;
}

PSOutput PS_Main(PSInput input) : SV_TARGET
{
    // float3 normalWS = normalize(input.normalWS);
    float3 col = float3(input.uv, 0);

    PSOutput output;
    output.Target0 = WriteGBuffer0(col, PIXEL_TYPE_SKYBOX);
    output.Target1 = WriteGBuffer1(float3(0,0,1));
    output.Target2 = WriteGBuffer2(input.positionSS.z / input.positionSS.w);

    return output;
}


