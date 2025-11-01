#include "shaders/lib/common.hlsl"

struct ParallelLightInfo
{
    float3 direction;
    float3 color;
    float shadowAttenuation;
};

float GetShadowAttenuation(float3 positionWS)
{
    float4 positionShadowSpace = mul(float4(positionWS, 1.0f), _MainLightShadowVP);
    positionShadowSpace.xyz = positionShadowSpace.xyz / positionShadowSpace.w;
    float2 shadowUV = positionShadowSpace.xy * 0.5f + 0.5f;
    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f || shadowUV.y < 0.0f || shadowUV.y > 1.0f)
    {
        return 1.0f;
    }

    float objectDepth = positionShadowSpace.z;
    float shadowDepth = _MainLightShadowMapTex.Sample(_MainLightShadowMapTexSampler, shadowUV).x * 2.0f - 1.0f;
    shadowDepth += 0.001f;

    return objectDepth < shadowDepth ? 1.0f : 0.5f;
    // return objectDepth < shadowDepth ? 1.0f : 0.0f;
}

ParallelLightInfo GetParallelLightInfo(int index, float3 positionWS)
{
    int startIndex = index * PARALLEL_LIGHT_INFO_STRIDE_VEC4;

    ParallelLightInfo l;
    l.direction = _ParallelLightInfo[startIndex].xyz;
    l.color = _ParallelLightInfo[startIndex + 1].xyz;
    l.shadowAttenuation = GetShadowAttenuation(positionWS);

    return l;
}

float3 Lit(float3 positionWS, float3 normalWS, float3 albedo)
{
    ParallelLightInfo mainLight = GetParallelLightInfo(0, positionWS);
    float3 finalColor = dot(mainLight.direction, normalWS) * 0.5f + 0.5f;
    finalColor *= mainLight.color * mainLight.shadowAttenuation * albedo / 3.1415926f;

    return finalColor;
}