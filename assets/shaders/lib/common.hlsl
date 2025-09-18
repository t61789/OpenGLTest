#ifndef COMMON_HLSL_INCLUDED
#define COMMON_HLSL_INCLUDED

#define PIXEL_TYPE_SKYBOX 1.0f
#define PIXEL_TYPE_LIT 2.0f
#define MAX_PARALLEL_LIGHT_COUNT 4
#define MAX_POINT_LIGHT_COUNT 16
#define PARALLEL_LIGHT_INFO_STRIDE_VEC4 2
#define POINT_LIGHT_INFO_STRIDE_VEC4 2

#define TEXTURE2D(name) Texture2D name; SamplerState name##Sampler;
#define TEXTURECUBE(name) TextureCube name; SamplerState name##Sampler;

cbuffer _Global : register(b0)
{
    float _FogIntensity;
    float4 _FogColor;
    float3 _MainLightDirection;

    float4 _ParallelLightInfo[MAX_PARALLEL_LIGHT_COUNT * PARALLEL_LIGHT_INFO_STRIDE_VEC4];
    float4 _PointLightInfo[MAX_POINT_LIGHT_COUNT * POINT_LIGHT_INFO_STRIDE_VEC4];
};

cbuffer PerViewCBuffer : register(b1)
{
    float4x4 _VP;
};

cbuffer PerObjectCBuffer : register(b2)
{
    float4x4 _M;
    float4x4 _IM;
    float4 _CameraPositionWS;
    float4 _ViewportSize;
};

cbuffer ObjectIndexCBuffer : register(b3)
{
    uint _ObjectIndex;
};

struct ObjectMatrix
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
};

StructuredBuffer<ObjectMatrix> _ObjectMatrices : register(t4);

struct VSInput 
{
    float4 positionOS : POSITION;
    float4 normalOS : NORMAL;
    float4 tangentOS : TANGENT;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    uint id : SV_InstanceID;
};

struct PSInput
{
    float4 positionCS : SV_POSITION;
    float4 positionSS : TEXCOORD0;
    float3 normalWS : TEXCOORD1;
    float2 uv : TEXCOORD2;
};

struct PSOutput
{
    float4 Target0 : SV_Target0;
    float4 Target1 : SV_Target1;
    float4 Target2 : SV_Target2;
};

TEXTURE2D(_GBuffer0Tex)
TEXTURE2D(_GBuffer1Tex)
TEXTURE2D(_GBuffer2Tex)
TEXTURE2D(_ShadingBufferTex)

TEXTURECUBE(_SkyboxTex)


float4 WriteGBuffer0(float3 albedo)
{
    return float4(albedo, PIXEL_TYPE_LIT);
}

float4 WriteGBuffer0(float3 albedo, float pixelType)
{
    return float4(albedo, pixelType);
}

float4 WriteGBuffer1(float3 normalWS)
{
    return float4(normalWS * 0.5 + 0.5, 0.0f);
}

float4 WriteGBuffer2(float depth)
{
    return float4(depth, 0.0f, 0.0f, 0.0f);
}

void ReadGBuffer0(float2 screenUV, out float3 albedo)
{
    float4 color = _GBuffer0Tex.Sample(_GBuffer0TexSampler, screenUV);
    albedo = color.xyz;
}

void ReadGBuffer0(float2 screenUV, out float3 albedo, out float pixelType)
{
    float4 color = _GBuffer0Tex.Sample(_GBuffer0TexSampler, screenUV);
    albedo = color.xyz;
    pixelType = color.w;
}

void ReadGBuffer1(float2 screenUV, out float3 normalWS)
{
    float4 color = _GBuffer1Tex.Sample(_GBuffer1TexSampler, screenUV);
    normalWS = color.xyz * 2 - 1;
}

void ReadGBuffer2(float2 screenUV, out float depth)
{
    float4 color = _GBuffer2Tex.Sample(_GBuffer2TexSampler, screenUV);
    depth = color.r;
}

float4x4 GetLocalToWorld()
{
    return _ObjectMatrices[_ObjectIndex].localToWorld;
}

float4x4 GetWorldToLocal()
{
    return _ObjectMatrices[_ObjectIndex].worldToLocal;
}

float4 TransformObjectToHClip(float3 positionOS, float4x4 localToWorld)
{
    return mul(mul(float4(positionOS, 1.0f), localToWorld), _VP);
}

float4 TransformObjectToHClip(float3 positionOS)
{
    return TransformObjectToHClip(positionOS, GetLocalToWorld());
}

float4 TransformWorldToHClip(float3 positionWS)
{
    return mul(float4(positionWS, 1.0f), _VP);
}

float3 TransformObjectToWorldNormal(float3 normalOS, float4x4 worldToLocal)
{
    return mul(transpose(worldToLocal), float4(normalOS, 0.0f)).xyz;
}

float3 TransformObjectToWorldNormal(float3 normalOS)
{
    return TransformObjectToWorldNormal(normalOS, GetWorldToLocal());
}

float4 SampleSkybox(float3 direction)
{
    return _SkyboxTex.Sample(_SkyboxTexSampler, direction);
}

#endif // COMMON_HLSL_INCLUDED
