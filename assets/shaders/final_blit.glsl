#version 330 core

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPositionOS.xy, 0, 1);
    texCoord = aPositionOS.xy * 0.5 + 0.5;
};

#version 330 core

#include "shaders/lib/common.glsl"

in vec2 texCoord;

uniform sampler2D _ShadingBufferTex;
uniform sampler2D _LutTex;

uniform float _ExposureMultiplier;
uniform float _MinLuminance;
uniform float _MaxLuminance;

out vec4 FragColor;

vec3 applyToneMapping(vec3 inputColor)
{
    const mat3 preTonemappingTransform = mat3(
        0.575961650, 0.070806820, 0.028035252,
        0.344143820, 0.827392350, 0.131523770,
        0.079952030, 0.101774690, 0.840242300
    );

    const mat3 postTonemappingTransform = mat3(
        1.666954300, -0.106835220, -0.004142626,
        -0.601741150,  1.237778600, -0.087411870,
        -0.065202855, -0.130948950,  1.091555000
    );

    mat3 exposedPreTonemappingTransform = _ExposureMultiplier * preTonemappingTransform;

    vec3 color = vec3(exposedPreTonemappingTransform * inputColor);

    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0, 1);

    color = vec3(postTonemappingTransform * color);

    return color;
}

vec3 applyLut(vec3 col)
{
    int offsetCount = int(col.b * 32.0);
    col.x = col.x / 32.0 + float(offsetCount) * (1.0 / 32.0);
    col.y = 1 - col.y;
    return texture(_LutTex, col.xy).rgb;
}

vec3 AdjustLuminance(vec3 color)
{
    // 计算当前颜色的亮度
    float currentLuminance = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;

    if(currentLuminance < _MinLuminance)
    {
        return vec3(0);
    }
    else if (currentLuminance > _MaxLuminance)
    {
        return vec3(1);
    }

    return color;
}

void main()
{
    vec4 color = texture(_ShadingBufferTex, texCoord);
    
    color.rgb = AdjustLuminance(color.rgb);
    
//    color.rgb = applyToneMapping(color.rgb);
    
//    color.rgb = applyLut(color.rgb);

    FragColor = vec4(color.rgb, 1);
};
