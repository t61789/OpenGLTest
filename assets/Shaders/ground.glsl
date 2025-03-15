#version 330 core

#include "shaders/lib/common.glsl"

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec4 vertexColor;
out vec2 texCoord;
smooth out highp vec3 normalWS;
out vec3 positionWS;
out vec4 positionCS;
out vec3 normalOS;

void main()
{
    gl_Position = TransformObjectToHClip(aPositionOS);
    positionCS = gl_Position;
    normalWS = TransformObjectToWorldNormal(normalize(aNormalOS));
    normalOS = aNormalOS;
    positionWS = TransformObjectToWorld(aPositionOS);
    vertexColor = vec4(aColor, 1.0);
    texCoord = aTexcoord;
}

#version 330 core

#include "shaders/lib/common.glsl"

in vec4 vertexColor;
in vec2 texCoord;
smooth in highp vec3 normalWS;
in vec3 positionWS;
in vec4 positionCS;
in vec3 normalOS;

uniform vec4 _EdgeColor;
uniform vec4 _GroundColor;
uniform float _GridSize;
uniform float _GridEdgeWidth;
uniform float _GridEdgeCrossWidth;
uniform float _GridEdgeCrossSize;

layout(location = 0) out vec4 FragColor0;
layout(location = 1) out vec4 FragColor1;
layout(location = 2) out vec4 FragColor2;

float GetGridFactor(vec3 positionWS)
{
    vec2 gridPos = mod(positionWS.xz / _GridSize, 1.0);
    
    float minX = min(gridPos.x, 1 - gridPos.x);
    float minY = min(gridPos.y, 1 - gridPos.y);
    float crossFactor = max(step(_GridEdgeCrossSize, minX), step(_GridEdgeCrossSize, minY));
    
    float factor = min(gridPos.x, min(1 - gridPos.x, min(gridPos.y, 1 - gridPos.y)));
    factor = step(mix(_GridEdgeWidth, _GridEdgeCrossWidth, crossFactor), factor);
    
    // 避免远处的摩尔纹
    float deltaLength = length(dFdx(positionWS)) + length(dFdy(positionWS));
    factor = mix(1, factor, 1 / (deltaLength * 5 + 1));
    
    return factor;
}

void main()
{
    float factor = GetGridFactor(positionWS);
    vec3 albedo = mix(_EdgeColor, _GroundColor, factor).xyz;
    if (factor > 0.5)
    {
        discard;
    }

    FragColor0 = WriteGBuffer0(albedo, PIXEL_TYPE_LIT);
    FragColor1 = WriteGBuffer1(normalize(normalWS));
    FragColor2 = WriteGBuffer2(positionCS.z / positionCS.w);
}
