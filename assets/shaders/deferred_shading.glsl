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
uniform float _FogIntensity;
uniform vec4 _FogColor;

out vec4 FragColor;

vec3 ApplyFog(vec3 albedo, vec3 positionWS)
{
    float dir = length(GetCameraPositionWS() - positionWS);
    float factor = exp(-_FogIntensity * dir);
    return mix(_FogColor.rgb, albedo, factor);
}

void main()
{
    vec3 albedo;
    float pixelType;
    vec3 normalWS;
    ReadGBuffer0(texCoord, albedo, pixelType);
    ReadGBuffer1(texCoord, normalWS);
    vec3 positionWS = TransformScreenToWorld(texCoord);

    //albedo = vec3(mod(positionWS.xz, 1.0), 0.0);

    vec3 finalColor;
    if(pixelType == PIXEL_TYPE_LIT)
    {
        finalColor = LitWithLights(normalWS, positionWS, albedo, 0.8, 0.0);
        //finalColor = normalWS;
        // finalColor = albedo;
        // finalColor = vec3(mod(abs(positionWS.xz), 1.0), 0.0);
    }
    else
    {
        finalColor = albedo;
    }

    //finalColor = ApplyFog(finalColor, positionWS);

    FragColor = vec4(finalColor, 1);
    // FragColor = vec4(texCoord, 0, 1);
//    FragColor = vec4(albedo, 1);
//    FragColor = vec4(mainLightShadowAttenuation);
//    FragColor = vec4(mod(positionWS.xz, 1), 0, 1);
}
