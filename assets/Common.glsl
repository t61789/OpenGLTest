uniform mat4 _MVP;
uniform mat4 _ITM;
uniform mat4 _M;

uniform vec4 _CameraPositionWS;
uniform vec4 _MainLightDirection;
uniform vec4 _MainLightColor;
uniform vec4 _AmbientLightColor;

vec3 TransformObjectToHClip(vec3 positionOS)
{
    return _MVP * vec4(positionOS, 1);
}

vec3 TransformObjectToWorldNormal(vec3 normalOS)
{
    return normalize((_ITM * vec4(aNormalOS, 0)).xyz);
}

vec3 TransformObjectToWorld(vec3 positionOS)
{
    return (_M * vec4(positionOS, 1)).xyz;
}

vec3 GetCameraPositionWS()
{
    return _CameraPositionWS.xyz;
}
