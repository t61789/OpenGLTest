#version 330 core

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPositionOS.xy, 0, 1);
    texCoord = aTexcoord;
}

#version 330 core

#include "shaders/lib/common.glsl"
#include "shaders/lib/lighting.glsl"

in vec2 texCoord;

uniform float _ExposureMultiplier;

out vec4 FragColor;

void main()
{
    vec3 albedo;
    float pixelType;
    vec3 normalWS;
    ReadGBuffer0(texCoord, albedo, pixelType);
    ReadGBuffer1(texCoord, normalWS);
    vec3 positionWS = TransformScreenToWorld(texCoord);
    
    vec3 finalColor;
    if(pixelType == PIXEL_TYPE_LIT)
    {
        finalColor = LitWithParallelLights(normalWS, positionWS, albedo, 1.0, 0.0);
    }
    else
    {
        finalColor = albedo;
    }

    FragColor = vec4(finalColor, 1);
//    FragColor = vec4(albedo, 1);
//    FragColor = vec4(mainLightShadowAttenuation);
//    FragColor = vec4(mod(positionWS.xz, 1), 0, 1);
}
