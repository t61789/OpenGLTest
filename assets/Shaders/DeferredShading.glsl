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
};

#version 330 core

#include "./Common.glsl"

in vec2 texCoord;

uniform float _ExposureMultiplier;

out vec4 FragColor;

void main()
{
    vec3 albedo;
    vec3 normalWS;
    ReadGBuffer0(texCoord, albedo);
    ReadGBuffer1(texCoord, normalWS);

//    vec3 viewDir = normalize(GetCameraPositionWS() - positionWS);

    vec3 ambient = _AmbientLightColor.rgb;
    vec3 diffuse = _MainLightColor.rgb * max(dot(normalWS, _MainLightDirection.xyz), 0);
//    vec3 H = (_MainLightDirection.xyz + viewDir) * 0.5;
//    vec3 specular = _MainLightColor.rgb * pow(max(dot(normalWS, H), 0), 20) * 5;
    vec3 specular = vec3(0);

    vec3 finalColor = diffuse + specular + ambient;

    FragColor = vec4(finalColor, 1);
};
