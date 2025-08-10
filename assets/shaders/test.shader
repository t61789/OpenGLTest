struct VSInput 
{
    float4 positionOS : POSITION;
    float3 tangentOS : TANGENT;
    float3 normalOS : NORMAL;
    float2 uv0 : TEXCOORD0;
};

struct VSOutput
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer _Global : register(b0)
{
    float4 _Albedo;
};


#include "shaders/sss.hlsl"

VSOutput VS_Main(VSInput input)
{
    VSOutput output;

    output.positionCS = float4(input.positionOS.xy, _Albedo.z, 1.0f);
    output.uv = input.uv0;

    return output;
}

cbuffer PerObjectCBuffer : register(b1)
{
    float4 _Albedo1;
};

struct PSOutput
{
    float4 Target0 : SV_Target0;  // 第一个渲染目标
    float4 Target1 : SV_Target1;  // 第二个渲染目标
    float4 Target2 : SV_Target2;  // 第三个渲染目标
    // 可以继续添加更多目标...
};

Texture2D _MainTex;
SamplerState _MainTexSampler;
Texture2D _MainTex0;
SamplerState _MainTex0Sampler;

PSOutput PS_Main(VSOutput input) : SV_TARGET
{
    PSOutput output;
    output.Target0 = _Albedo * _MainTex.Sample(_MainTexSampler, input.uv); // 红色
    output.Target1 = _Albedo1; // 绿色
    output.Target2 = float4(0.0f, 0.0f, 1.0f, 1.0f); // 蓝色

    return output;
}
