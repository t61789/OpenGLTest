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
    
    vec3 positionWS = TransformScreenToWorld(texCoord);
    
    float mainLightShadowAttenuation = SampleShadowMap(positionWS);
    
    vec3 mainLightColor = _MainLightColor.rgb * mainLightShadowAttenuation;

    vec3 viewDir = normalize(GetCameraPositionWS() - positionWS);

    vec3 ambient = _AmbientLightColor.rgb;
    vec3 diffuse = mainLightColor * albedo * max(dot(normalWS, _MainLightDirection.xyz), 0);
    vec3 H = normalize(_MainLightDirection.xyz + viewDir);
    vec3 specular = mainLightColor * albedo * pow(max(dot(normalWS, H), 0), 20) * 5;

    vec3 finalColor = diffuse + specular + ambient;

    FragColor = vec4(finalColor, 1);
//    FragColor = vec4(mainLightShadowAttenuation);
//    FragColor = vec4(mod(positionWS.xz, 1), 0, 1);
}
