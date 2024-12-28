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
}

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

layout(location = 0) out vec4 FragColor0;
layout(location = 1) out vec4 FragColor1;
layout(location = 2) out vec4 FragColor2;

void main()
{
   vec3 albedo = _Albedo.rgb;
   if(_ShowTex > 0.5)
   {
      albedo *= texture(_MainTex, texCoord).rgb;
   }

   FragColor0 = WriteGBuffer0(albedo, PIXEL_TYPE_LIT);
   FragColor1 = WriteGBuffer1(normalWS);
   FragColor2 = WriteGBuffer2(positionCS.z / positionCS.w);
}
