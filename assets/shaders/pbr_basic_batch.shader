#include "shaders/lib/common.hlsl"
#include "shaders/lib/batch_rendering.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionCS = TransformObjectToHClipBatch(input.positionOS.xyz, input.id);
    output.positionSS = output.positionCS;
    output.normalWS = TransformObjectToWorldNormalBatch(input.normalOS.xyz, input.id);
    output.uv = input.uv0;

    return output;
}

Texture2D _MainTex;
SamplerState _MainTexSampler;

PSOutput PS_Main(PSInput input) : SV_TARGET
{
    float4 albedo = _MainTex.Sample(_MainTexSampler, input.uv);
    float3 normalWS = normalize(input.normalWS);

    PSOutput output;
    output.Target0 = WriteGBuffer0(albedo.rgb);
    output.Target1 = WriteGBuffer1(normalWS);
    output.Target2 = WriteGBuffer2(input.positionSS.z / input.positionSS.w);

    return output;
}
