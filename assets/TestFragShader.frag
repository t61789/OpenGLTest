#version 330 core

in vec4 vertexColor;
in vec2 texCoord;
in vec3 normalWS;
in vec3 positionWS;

uniform sampler2D _MainTex;
uniform float _ShowTex;
uniform vec4 _CameraPositionWS;

out vec4 FragColor;

void main()
{
   if(_ShowTex > 0.5)
   {
      vec4 texCol = texture(_MainTex, texCoord);
      FragColor = vec4(texCol.rgb, 1);
   }
   else
   {
      vec3 viewDir = normalize(_CameraPositionWS.xyz - positionWS);
      float d = clamp(dot(viewDir, normalWS), 0, 1);
      FragColor = vec4(d, d, d, 1);
   }
};