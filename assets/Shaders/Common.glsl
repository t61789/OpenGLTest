#define PIXEL_TYPE_SKYBOX 1
#define PIXEL_TYPE_LIT 2

#define PI 3.141592657
#define SMALL 0.001

uniform mat4 _MVP;
uniform mat4 _ITM;
uniform mat4 _M;
uniform mat4 _IVP;
uniform mat4 _MainLightShadowVP;

uniform vec4 _CameraPositionWS;
uniform vec4 _MainLightDirection;
uniform vec4 _MainLightColor;
uniform vec4 _AmbientLightColor;

uniform sampler2D _MainLightShadowMapTex;

uniform sampler2D _GBuffer0Tex;
uniform sampler2D _GBuffer1Tex;
uniform sampler2D _GBuffer2Tex;
uniform samplerCube _SkyboxTex;

vec4 WriteGBuffer0(vec3 albedo)
{
    return vec4(albedo, 1);
}

vec4 WriteGBuffer0(vec3 albedo, int pixelType)
{
    return vec4(albedo, (float(pixelType) / float(255)));
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

void ReadGBuffer0(vec2 screenUV, out vec3 albedo, out int pixelType)
{
    vec4 color = texture(_GBuffer0Tex, screenUV);
    albedo = color.xyz;
    pixelType = int(color.w * float(255));
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
    float objectDepth = positionShadowSpace.z;
//    float bias = max(0.05 * (1 - dot(positionWS, _MainLightDirection.xyz)), 0.005);
    float bias = 0.001;
    return (shadowDepth + bias) < objectDepth ? 0 : 1;
}

vec3 GetCameraPositionWS()
{
    return _CameraPositionWS.xyz;
}

vec3 SampleSkybox(vec3 dir)
{
    return texture(_SkyboxTex, dir).rgb;
}

float square(float a)
{
    return a * a;
}

float Luminance(vec3 col)
{
    return 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
}
