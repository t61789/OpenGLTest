#if !defined(BATCH_RENDERING_HLSL_INCLUDED)
    #define BATCH_RENDERING_HLSL_INCLUDED

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

    #if defined(BATCH_RENDERING)
        #define TransformObjectToWorld(positionOS) TransformObjectToWorld(positionOS, GetBatchObjectInfo(input.id).localToWorld)
        #define TransformObjectToHClip(positionOS) TransformObjectToHClip(positionOS, GetBatchObjectInfo(input.id).localToWorld)
        #define TransformObjectToWorldNormal(normalOS) TransformObjectToWorldNormal(normalOS, GetBatchObjectInfo(input.id).worldToLocal)
    #endif

#endif // BATCH_RENDERING_HLSL_INCLUDED
