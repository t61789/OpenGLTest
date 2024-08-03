#version 330 core

in vec4 vertexColor;
in vec2 texCoord;

uniform sampler2D tex;

out vec4 FragColor;

void main()
{
   vec4 texCol = texture(tex, texCoord);
   FragColor = texCol;
};