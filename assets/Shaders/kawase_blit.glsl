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

#include "./common.glsl"

in vec2 texCoord;

uniform sampler2D _MainTex;
uniform vec4 _MainTex_TexelSize;
uniform float _Iterations;
uniform vec4 _TargetTexelSize;

out vec4 FragColor;

void main()
{
    vec2 center = texCoord;
    vec2 uv0 = center + (vec2(-0.5, -0.5) + _Iterations * vec2(-1.0, -1.0)) * _MainTex_TexelSize.xy;
    vec2 uv1 = center + (vec2(0.5, -0.5) + _Iterations * vec2(1.0, -1.0)) * _MainTex_TexelSize.xy;
    vec2 uv2 = center + (vec2(-0.5, 0.5) + _Iterations * vec2(-1.0, 1.0)) * _MainTex_TexelSize.xy;
    vec2 uv3 = center + (vec2(0.5, 0.5) + _Iterations * vec2(1.0, 1.0)) * _MainTex_TexelSize.xy;

    vec3 color = texture(_MainTex, center).rgb;
    vec3 color0 = texture(_MainTex, uv0).rgb;
    vec3 color1 = texture(_MainTex, uv1).rgb;
    vec3 color2 = texture(_MainTex, uv2).rgb;
    vec3 color3 = texture(_MainTex, uv3).rgb;

    vec3 col = (color + color0 + color1 + color2 + color3) / 5.0;

    FragColor = vec4(col, 1);
};
