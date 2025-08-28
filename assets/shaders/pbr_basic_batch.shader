#include "shaders/lib/common.hlsl"

StructuredBuffer<uint> _BatchMatricesIndices : register(t4);
StructuredBuffer<ObjectMatrix> _BatchMatrices : register(t5);

PSInput VS_Main(VSInput input)
{
    PSInput output;

    float4x4 localToWorld = _BatchMatrices[_BatchMatricesIndices[input.id]].localToWorld;
    output.positionCS = mul(mul(float4(input.positionOS.xyz, 1.0f), localToWorld), _VP);
    output.positionSS = output.positionCS;
    output.normalWS = TransformObjectToWorldNormal(input.normalOS.xyz);
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
