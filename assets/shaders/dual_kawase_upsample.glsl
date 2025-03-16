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

out vec4 FragColor;

void main()
{
    vec2 center = texCoord;
    vec2 targetTexelSize = _MainTex_TexelSize.xy * 0.5;
    vec2 uv0 = center + vec2(-2, 0) * targetTexelSize * _BlurSize;
    vec2 uv1 = center + vec2(0, -2) * targetTexelSize * _BlurSize;
    vec2 uv2 = center + vec2(2, 0) * targetTexelSize * _BlurSize;
    vec2 uv3 = center + vec2(0, 2) * targetTexelSize * _BlurSize;

    vec2 uv4 = center + vec2(-1, -1) * targetTexelSize * _BlurSize;
    vec2 uv5 = center + vec2(1, -1) * targetTexelSize * _BlurSize;
    vec2 uv6 = center + vec2(-1, 1) * targetTexelSize * _BlurSize;
    vec2 uv7 = center + vec2(1, 1) * targetTexelSize * _BlurSize;

    vec3 color0 = texture(_MainTex, uv0).rgb;
    vec3 color1 = texture(_MainTex, uv1).rgb;
    vec3 color2 = texture(_MainTex, uv2).rgb;
    vec3 color3 = texture(_MainTex, uv3).rgb;

    vec3 color4 = texture(_MainTex, uv4).rgb;
    vec3 color5 = texture(_MainTex, uv5).rgb;
    vec3 color6 = texture(_MainTex, uv6).rgb;
    vec3 color7 = texture(_MainTex, uv7).rgb;

    vec3 col = ((color0 + color1 + color2 + color3) * 2.0 + color4 + color5 + color6 + color7) / 12.0;

    FragColor = vec4(col, 1);
};
