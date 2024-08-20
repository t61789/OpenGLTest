#version 330 core

in vec4 vertexColor;
in vec2 texCoord;
in vec3 normalWS;

uniform sampler2D _MainTex;
uniform float _ShowTex;

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
      FragColor = vec4(normalize(normalWS), 1);
   }
};