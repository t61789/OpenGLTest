#include "shaders/lib/common.hlsl"

struct ParallelLightInfo
{
    float3 direction;
    float3 color;
    float shadowAttenuation;
};

float GetShadowAttenuation(float3 positionWS)
{
    float4 shadowCoord = mul(float4(positionWS, 1.0f), _MainLightShadowVP);
    shadowCoord.xyz = shadowCoord.xyz / shadowCoord.w;
    shadowCoord.xy = shadowCoord.xy * 0.5f + 0.5f;
    if (shadowCoord.x < 0.0f || shadowCoord.x > 1.0f || shadowCoord.y < 0.0f || shadowCoord.y > 1.0f || shadowCoord.z < 0.0f || shadowCoord.z > 1.0f)
    {
        return 1.0f;
    }

    float objectDepth = shadowCoord.z;
    float shadowDepth = _MainLightShadowMapTex.Sample(_MainLightShadowMapTexSampler, shadowCoord.xy).x * 2.0f - 1.0f;
    shadowDepth += 0.001f;

    float shadowAttenuation = objectDepth < shadowDepth ? 1.0f : 0.5f;
    float shadowFade = saturate((_MainLightShadowRange - length(positionWS - _CameraPositionWS.xyz)) / 5.0f);
    shadowAttenuation = lerp(1.0f, shadowAttenuation, shadowFade);

    return shadowAttenuation;
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