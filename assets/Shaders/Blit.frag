#version 330 core

#include "Common.glsl"

in vec2 texCoord;

uniform sampler2D _MainTex;

out vec4 FragColor;

void main()
{
   FragColor = vec4(texCoord.xy, 0, 1);
};