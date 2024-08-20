#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aColor;

uniform mat4 _MVP;
uniform mat4 _ITM;
uniform mat4 _M;

out vec4 vertexColor;
out vec2 texCoord;
out vec3 normalWS;

void main()
{
   gl_Position = _MVP * vec4(aPos, 1);
   vertexColor = vec4(aColor, 1.0);
   texCoord = aTexCoord;
   normalWS = (_ITM * vec4(aNormal, 0)).xyz;
};