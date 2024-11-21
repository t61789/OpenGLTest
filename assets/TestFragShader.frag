#version 330 core

#include "Common.glsl"

in vec4 vertexColor;
in vec2 texCoord;
in vec3 normalWS;
in vec3 positionWS;

uniform sampler2D _MainTex;
uniform float _ShowTex;
uniform vec4 _Albedo;

out vec4 FragColor;

void main()
{
   vec3 albedo = _Albedo.rgb;
   if(_ShowTex > 0.5)
   {
      albedo *= texture(_MainTex, texCoord).rgb;
   }
   
   normalWS = normalize(normalWS);
   
   vec3 viewDir = normalize(GetCameraPositionWS() - positionWS);
   
   vec3 ambient = _AmbientLightColor.rgb;
   vec3 diffuse = _MainLightColor.rgb * _Albedo.rgb * max(dot(normalWS, _MainLightDirection.xyz), 0);
   vec3 H = (_MainLightDirection.xyz + viewDir) * 0.5;
   vec3 specular = _MainLightColor.rgb * pow(max(dot(normalWS, H), 0), 3);
   
   vec3 finalColor = diffuse + specular + ambient;
           
   FragColor = vec4(finalColor, 1);
};