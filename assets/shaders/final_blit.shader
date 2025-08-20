#include "shaders/lib/common.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.positionCS = float4(input.positionOS.xy, 0.0f, 1.0f);
    output.uv = input.positionOS.xy * 0.5f + 0.5f;

    return output;
}

TEXTURE2D(_LutTex)

cbuffer PerMaterialCBuffer : register(b3)
{
    float _ExposureMultiplier;
    float _MinLuminance;
    float _MaxLuminance;
};

float3 ApplyLut(float3 col)
{
    int offsetCount = int(col.b * 32.0f);
    col.x = col.x / 32.0f + float(offsetCount) * (1.0f / 32.0f);
    col.y = 1 - col.y;
    return _LutTex.Sample(_LutTexSampler, col.xy).rgb;
}

float3 AdjustLuminance(float3 color)
{
    // 计算当前颜色的亮度
    float currentLuminance = 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;

    if (currentLuminance < _MinLuminance)
    {
        return float3(0);
    }
    else if (currentLuminance > _MaxLuminance)
    {
        return float3(1);
    }

    return color;
}

float4 PS_Main(PSInput input) : SV_Target
{
    float4 color = _ShadingBufferTex.Sample(_ShadingBufferTexSampler, input.uv);
    
    // color.rgb = AdjustLuminance(color.rgb);
    
    // color.rgb = applyToneMapping(color.rgb);
    
//    color.rgb = applyLut(color.rgb);

    return float4(color.rgb, 1);
}
