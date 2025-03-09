#version 330 core

#include "shaders/lib/common.glsl"

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec4 positionCS;

void main()
{
    gl_Position = TransformObjectToHClip(aPositionOS);
    positionCS = gl_Position;
}

#version 330 core

#include "shaders/lib/common.glsl"

in vec4 positionCS;

layout(location = 0) out vec4 FragColor0;

void main()
{
    FragColor0 = vec4(positionCS.z / positionCS.w, 0, 0, 0);
}
