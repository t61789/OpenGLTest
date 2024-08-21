#version 330 core

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

uniform mat4 _MVP;
uniform mat4 _ITM;
uniform mat4 _M;

out vec4 vertexColor;
out vec2 texCoord;
out vec3 normalWS;
out vec3 positionWS;

void main()
{
   gl_Position = _MVP * vec4(aPositionOS, 1);
   vertexColor = vec4(aColor, 1.0);
   texCoord = aTexcoord;
   normalWS = normalize((_ITM * vec4(aNormalOS, 0)).xyz);
   positionWS = (_M * vec4(aPositionOS, 1)).xyz;
};