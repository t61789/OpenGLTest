uniform mat4 _MVP;
uniform mat4 _ITM;
uniform mat4 _M;

uniform vec4 _CameraPositionWS;
uniform vec4 _MainLightDirection;
uniform vec4 _MainLightColor;
uniform vec4 _AmbientLightColor;

uniform sampler2D _GBuffer0Rt;
uniform sampler2D _GBuffer1Rt;

vec4 TransformObjectToHClip(vec3 positionOS)
{
    return _MVP * vec4(positionOS, 1);
}

vec3 TransformObjectToWorldNormal(vec3 normalOS)
{
    return normalize((_ITM * vec4(normalOS, 0)).xyz);
}

vec3 TransformObjectToWorld(vec3 positionOS)
{
    return (_M * vec4(positionOS, 1)).xyz;
}

vec3 GetCameraPositionWS()
{
    return _CameraPositionWS.xyz;
}

vec4 WriteGBuffer0(vec3 albedo)
{
    return vec4(albedo, 1);
}

vec4 WriteGBuffer1(vec3 normalWS)
{
    return vec4(normalWS.xyz * 0.5 + 0.5, 1);
}

void ReadGBuffer0(vec2 screenUV, out vec3 albedo)
{
    vec4 color = texture(_GBuffer0Rt, screenUV);
    albedo = color.xyz;
}

void ReadGBuffer1(vec2 screenUV, out vec3 normalWS)
{
    vec4 color = texture(_GBuffer1Rt, screenUV);
    normalWS = color.xyz * 2 - 1;
}
