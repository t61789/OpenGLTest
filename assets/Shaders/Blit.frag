#version 330 core

in vec2 texCoord;

uniform sampler2D _MainTex;

out vec4 FragColor;

void main()
{
   FragColor = texture(_MainTex, texCoord);
};