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

uniform sampler2D _MainTex;
uniform vec4 _MainTex_TexelSize;

uniform float _BlurSize;
uniform float _ApplyThreshold;
uniform float _Threshold;

out vec4 FragColor;

vec3 SampleColor(vec2 center, vec2 offset)
{
    vec2 uv = center + offset * _MainTex_TexelSize.xy * _BlurSize;
    vec3 color = texture(_MainTex, uv).rgb;

    if (_ApplyThreshold > 0)
    {
        float lume = Luminance(color);
        if (lume < _Threshold)
        {
            color = vec3(0);
        }
    }

    return color;
}

void main()
{
    vec2 center = texCoord;

    vec3 color = SampleColor(center, vec2(0, 0));
    vec3 color0 = SampleColor(center, vec2(-1, -1));
    vec3 color1 = SampleColor(center, vec2(1, -1));
    vec3 color2 = SampleColor(center, vec2(-1, 1));
    vec3 color3 = SampleColor(center, vec2(1, 1));

    vec3 col = (color * 4.0 + color0 + color1 + color2 + color3) / 8.0;

    FragColor = vec4(col, 1);
};
