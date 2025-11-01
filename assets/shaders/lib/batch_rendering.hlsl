#include "shaders/lib/common.hlsl"

struct BatchObjectInfo
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
};

StructuredBuffer<uint> _BatchObjectIndices : register(t5);
StructuredBuffer<BatchObjectInfo> _BatchObjectInfo : register(t6);


BatchObjectInfo GetBatchObjectInfo(uint id)
{
    return _BatchObjectInfo[_BatchObjectIndices[id]];
}

float3 TransformObjectToWorldBatch(float3 positionOS, uint id)
{
    return TransformObjectToWorld(positionOS, GetBatchObjectInfo(id).localToWorld);
}

float4 TransformObjectToHClipBatch(float3 positionOS, uint id)
{
    return TransformObjectToHClip(positionOS, GetBatchObjectInfo(id).localToWorld);
}

float3 TransformObjectToWorldNormalBatch(float3 normalOS, uint id)
{
    return TransformObjectToWorldNormal(normalOS, GetBatchObjectInfo(id).worldToLocal);
}
