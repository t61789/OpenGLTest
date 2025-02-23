#version 330 core

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPositionOS.xy, 0, 1);
    texCoord = aPositionOS.xy * 0.5 + 0.5;
};

#version 330 core

in vec2 texCoord;

uniform sampler2D _MainTex;

out vec4 FragColor;

void main()
{
    vec4 color = texture(_MainTex, texCoord);

    FragColor = vec4(color.rgb, 1);
};
