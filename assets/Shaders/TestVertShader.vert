#version 330 core

#include "Shaders/Common.glsl"

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec4 vertexColor;
out vec2 texCoord;
out vec3 normalWS;
out vec3 positionWS;

void main()
{
   gl_Position = TransformObjectToHClip(aPositionOS);
   normalWS = TransformObjectToWorldNormal(aNormalOS);
   positionWS = TransformObjectToWorld(aPositionOS);
   vertexColor = vec4(aColor, 1.0);
   texCoord = aTexcoord;
};