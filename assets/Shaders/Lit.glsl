#version 330 core

#include "./Common.glsl"

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec4 vertexColor;
out vec2 texCoord;
out vec3 normalWS;
out vec3 positionWS;
out vec4 positionCS;

void main()
{
   gl_Position = TransformObjectToHClip(aPositionOS);
   positionCS = gl_Position;
   normalWS = TransformObjectToWorldNormal(aNormalOS);
   positionWS = TransformObjectToWorld(aPositionOS);
   vertexColor = vec4(aColor, 1.0);
   texCoord = aTexcoord;
};

#version 330 core

#include "./Common.glsl"

in vec4 vertexColor;
in vec2 texCoord;
in vec3 normalWS;
in vec3 positionWS;
in vec4 positionCS;

uniform sampler2D _MainTex;
uniform float _ShowTex;
uniform vec4 _Albedo;

out vec4 FragColor0;
out vec4 FragColor1;
out vec4 FragColor2;

void main()
{
   vec3 albedo = _Albedo.rgb;
   if(_ShowTex > 0.5)
   {
      albedo *= texture(_MainTex, texCoord).rgb;
   }

   vec3 viewDir = normalize(GetCameraPositionWS() - positionWS);

   vec3 ambient = _AmbientLightColor.rgb;
   vec3 diffuse = _MainLightColor.rgb * _Albedo.rgb * max(dot(normalWS, _MainLightDirection.xyz), 0);
   vec3 H = (_MainLightDirection.xyz + viewDir) * 0.5;
   vec3 specular = _MainLightColor.rgb * pow(max(dot(normalWS, H), 0), 20) * 5;

   vec3 finalColor = diffuse + specular + ambient;

   FragColor0 = WriteGBuffer0(finalColor);
   FragColor1 = WriteGBuffer1(normalWS);
   FragColor0 = WriteGBuffer2(positionCS.z / positionCS.w);
};
