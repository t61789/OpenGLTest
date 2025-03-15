#define PIXEL_TYPE_SKYBOX 1.0
#define PIXEL_TYPE_LIT 2.0

#define PI 3.141592657
#define SMALL 0.0001

uniform mat4 _MVP;
uniform mat4 _ITM;
uniform mat4 _M;
uniform mat4 _VP;
uniform mat4 _IVP;
uniform mat4 _MainLightShadowVP;

uniform vec4 _CameraPositionWS;
uniform vec4 _MainLightDirection;
uniform vec4 _MainLightColor;
uniform vec4 _AmbientLightColor;

// uniform sampler2DShadow _MainLightShadowMapTex;
uniform sampler2D _MainLightShadowMapTex;

uniform sampler2D _GBuffer0Tex;
uniform sampler2D _GBuffer1Tex;
uniform sampler2D _GBuffer2Tex;
uniform samplerCube _SkyboxTex;

vec4 WriteGBuffer0(vec3 albedo)
{
    return vec4(albedo, 1);
}

vec4 WriteGBuffer0(vec3 albedo, float pixelType)
{
    return vec4(albedo, pixelType);
}

vec4 WriteGBuffer1(vec3 normalWS)
{
    return vec4(normalWS.xyz * 0.5 + 0.5, 1);
}

vec4 WriteGBuffer2(float depth)
{
    return vec4(depth, 0, 0, 0);
}

void ReadGBuffer0(vec2 screenUV, out vec3 albedo)
{
    vec4 color = texture(_GBuffer0Tex, screenUV);
    albedo = color.xyz;
}

void ReadGBuffer0(vec2 screenUV, out vec3 albedo, out float pixelType)
{
    vec4 color = texture(_GBuffer0Tex, screenUV);
    albedo = color.xyz;
    pixelType = color.w;
}

void ReadGBuffer1(vec2 screenUV, out vec3 normalWS)
{
    vec4 color = texture(_GBuffer1Tex, screenUV);
    normalWS = color.xyz * 2 - 1;
}

void ReadGBuffer2(vec2 screenUV, out float depth)
{
    vec4 color = texture(_GBuffer2Tex, screenUV);
    depth = color.r;
}

vec3 TransformObjectToWorld(vec3 positionOS)
{
    return (_M * vec4(positionOS, 1)).xyz;
}

vec3 TransformObjectToWorldNormal(vec3 normalOS)
{
    return normalize((_ITM * vec4(normalOS, 0)).xyz);
}

vec4 TransformWorldToHClip(vec3 positionWS)
{
    return _VP * vec4(positionWS, 1);
}

vec4 TransformObjectToHClip(vec3 positionOS)
{
    return _MVP * vec4(positionOS, 1);
}

vec3 TransformScreenToWorld(vec2 screenUV)
{
    float depth;
    ReadGBuffer2(screenUV, depth);
    vec4 positionCS = vec4(screenUV * 2 - 1, depth, 1);
    vec4 positionWS = _IVP * positionCS;
    
    return positionWS.xyz / positionWS.w;
}

float SampleShadowMap(vec3 positionWS)
{
    vec4 positionShadowSpace = _MainLightShadowVP * vec4(positionWS, 1);
    positionShadowSpace.xyz /= positionShadowSpace.w;
    vec2 shadowUV = positionShadowSpace.xy * 0.5 + 0.5;
    
    if(shadowUV.x < 0 || shadowUV.x > 1 || shadowUV.y < 0 || shadowUV.y > 1)
    {
        return 1;
    }
    
    float shadowDepth = texture(_MainLightShadowMapTex, shadowUV).r * 2 - 1;
    float objectDepth = clamp(positionShadowSpace.z, -1, 1);
    float bias = 0.005;
    // float bias = 0.05;
    // float bias = 0;
    // #ifdef FRAG_SHADER
    //     bias += max(abs(dFdx(objectDepth)), abs(dFdy(objectDepth))) * 5;
    // #endif
    // float bias = 0;
    return step(0, shadowDepth + bias - objectDepth);
}

vec3 GetCameraPositionWS()
{
    return _CameraPositionWS.xyz;
}

vec3 SampleSkybox(vec3 dir)
{
    return texture(_SkyboxTex, dir).rgb;
}

vec3 SampleSkybox(vec3 dir, float level)
{
    return textureLod(_SkyboxTex, dir, level).rgb;
}

float square(float a)
{
    return a * a;
}

float Luminance(vec3 col)
{
    return 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
}

float unlerp(float a, float b, float t)
{
    return (t - a) / (b - a);
}

vec3 Pow(vec3 col, float exp)
{
    vec3 result;   
    result.x = pow(col.x, exp);
    result.y = pow(col.y, exp);
    result.z = pow(col.z, exp);
    return result;
}
