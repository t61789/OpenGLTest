#include "shaders/lib/common.hlsl"

struct ParallelLightInfo
{
    float3 direction;
    float3 color;
};

ParallelLightInfo GetParallelLightInfo(int index)
{
    int startIndex = index * PARALLEL_LIGHT_INFO_STRIDE_VEC4;

    ParallelLightInfo l;
    l.direction = _ParallelLightInfo[startIndex].xyz;
    l.color = _ParallelLightInfo[startIndex + 1].xyz;

    return l;
}


float3 Lit(float3 positionWS, float3 normalWS, float3 albedo)
{
    ParallelLightInfo mainLight = GetParallelLightInfo(0);
    float3 finalColor = dot(mainLight.direction, normalWS) * 0.5 + 0.5;
    finalColor *= mainLight.color * albedo / 3.1415926;

    return finalColor;
}