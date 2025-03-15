#include "shaders/lib/common.glsl"
#include "shaders/lib/indirect_lighting.glsl"

#define MAX_PARALLEL_LIGHT_COUNT 4
#define MAX_POINT_LIGHT_COUNT 16

struct ParallelLightInfo
{
    vec3 direction;
    vec3 color;
};
#define PARALLEL_LIGHT_INFO_STRIDE 6
uniform float _ParallelLightInfo[6 * MAX_PARALLEL_LIGHT_COUNT];
uniform int _ParallelLightCount;

struct PointLightInfo
{
    vec3 positionWS;
    float radius;
    vec3 color;
};
#define POINT_LIGHT_INFO_STRIDE 7
uniform float _PointLightInfo[POINT_LIGHT_INFO_STRIDE * MAX_POINT_LIGHT_COUNT];
uniform int _PointLightCount;

struct Light
{
    vec3 color;
    vec3 direction;
};

Light GetParallelLight(int index) 
{
    Light l;
    int startOffset = index * PARALLEL_LIGHT_INFO_STRIDE;
    l.direction = vec3(
        _ParallelLightInfo[startOffset + 0], 
        _ParallelLightInfo[startOffset + 1], 
        _ParallelLightInfo[startOffset + 2]
    );
    l.color = vec3(
        _ParallelLightInfo[startOffset + 3], 
        _ParallelLightInfo[startOffset + 4], 
        _ParallelLightInfo[startOffset + 5]
    );
    return l;
}

Light GetPointLight(int index, vec3 positionWS)
{
    Light l;
    int startOffset = index * POINT_LIGHT_INFO_STRIDE;
    vec3 lightPositionWS = vec3(
        _PointLightInfo[startOffset + 0],
        _PointLightInfo[startOffset + 1],
        _PointLightInfo[startOffset + 2]
    );
    vec3 v = lightPositionWS - positionWS;
    l.direction = normalize(v);

    float radius = _PointLightInfo[startOffset + 3];

    float d = length(v);
    float i = 1 / (1 + radius * d);
    l.color = vec3(
        _PointLightInfo[startOffset + 4],
        _PointLightInfo[startOffset + 5],
        _PointLightInfo[startOffset + 6]
    );
    l.color *= i;

    return l;
}

vec3 GetF0(vec3 albedo, float metallic)
{
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    return F0;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 GGXSpecular(float roughness, vec3 normalWS, vec3 viewDir, vec3 F0, vec3 lightDirection)
{
    vec3 n = normalWS;
    vec3 v = viewDir;
    vec3 l = lightDirection;
    vec3 h = normalize(l + viewDir);
    float ndh = max(dot(n, h), SMALL);
    float ndv = max(dot(n, v), SMALL);
    float ndl = max(dot(n, l), SMALL);
    float ldh = max(dot(l, h), SMALL);
    
    float a = roughness;
    float d = square(a) / (PI * square(square(ndh) * (square(a) - 1) + 1));
    vec3 f = FresnelSchlick(ldh, F0);
    float g = 2 * ndv * ndl / (ndv + sqrt(square(a) + (1 - square(a)) * square(ndv)));
    vec3 s = f * g * d / (4 * ndl * ndv);
    
    return s;
}

vec3 Lit(vec3 normalWS, vec3 positionWS, Light light, float shadowAttenuation, vec3 albedo, float roughness, float metallic)
{
    vec3 lightColor = light.color * shadowAttenuation;
    vec3 viewDir = normalize(GetCameraPositionWS() - positionWS);
    vec3 ambient = _AmbientLightColor.rgb;

    vec3 l = light.direction;
    vec3 h = normalize(l + viewDir);
    vec3 n = normalWS;
    vec3 v = viewDir;
    float ldh = max(dot(l, h), SMALL);
    float ndl = max(dot(n, l), SMALL);
    float ndv = max(dot(n, v), SMALL);
    
    vec3 F0 = GetF0(albedo, metallic);
    vec3 f = FresnelSchlick(ldh, F0);
    vec3 kD = vec3(1.0) - f;
    kD *= vec3(1.0) - vec3(metallic);
    
    vec3 diffuse = albedo * ndl * lightColor;
    vec3 H = normalize(light.direction + viewDir);
    vec3 specular = GGXSpecular(roughness, normalWS, viewDir, F0, l) * lightColor;

    vec3 environmentReflection = SampleSkybox(reflect(-v, n), mix(0, 12, roughness)) * Luminance(_MainLightColor.rgb);
    vec3 eF = F0 + (1.0 - F0) * pow(max(1.0 - ndv, 0), 5.0);
    environmentReflection *= eF; // TODO 预计算
    environmentReflection = roughness > 0.95 ? CalcIndirectLighting(normalWS) : environmentReflection;
    environmentReflection = vec3(0); // TODO

    return kD * diffuse + specular + environmentReflection;
}

vec3 Lit0(vec3 normalWS, vec3 viewDirWS, Light light, float mainLightShadowAttenuation, vec3 albedo, float roughness,  float metallic)
{
    light.color *= mainLightShadowAttenuation;

    float a = roughness * roughness;
    vec3 h = normalize(light.direction + viewDirWS);
    float ndh = max(dot(normalWS, h), SMALL);
    float vdh = max(dot(viewDirWS, h), SMALL);
    float ndl = max(dot(normalWS, light.direction), SMALL);
    float ndv = max(dot(normalWS, viewDirWS), SMALL);

    float t0 = ndh * ndh * (a - 1) + 1;
    float D = a / (PI * t0 * t0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = F0 + (1 - F0) * pow(max(1 - vdh, 0), 5);

    float k = a * 0.5;
    t0 = ndl / (ndl * (1 - k) + k);
    float t1 = ndv / (ndv * (1 - k) + k);
    float G = t0 * t1;

    vec3 specular = D * F * G / max(4 * ndv * ndl, SMALL);

    vec3 diffuse = albedo / PI * (1 - F) * (1 - metallic);

    return (specular + diffuse) * light.color * ndl;
}

vec3 LitWithParallelLights(vec3 normalWS, vec3 positionWS, vec3 viewDirWS, vec3 albedo, float roughness, float metallic)
{
    vec3 result = vec3(0);
    float mainLightShadowAttenuation = SampleShadowMap(positionWS);
    for (int i = 0; i < _ParallelLightCount; i++)
    {
        Light light = GetParallelLight(i);
        mainLightShadowAttenuation = i == 0 ? mainLightShadowAttenuation : 1;
        result += Lit0(normalWS, viewDirWS, light, mainLightShadowAttenuation, albedo, roughness, metallic);
    }

    return result;
}

vec3 LitWithPointLights(vec3 normalWS, vec3 positionWS, vec3 viewDirWS, vec3 albedo, float roughness, float metallic)
{
    vec3 result = vec3(0);
    for (int i = 0; i < _ParallelLightCount; i++)
    {
        Light light = GetPointLight(i, positionWS);
        result += Lit0(normalWS, viewDirWS, light, 1, albedo, roughness, metallic);
    }

    return result;
}

vec3 LitWithIndirectLights(vec3 normalWS, vec3 viewDirWS, vec3 albedo, float roughness, float metallic)
{
    Light light;
    light.color = CalcIndirectLighting(normalWS);   
    light.direction = normalWS;

    return Lit0(normalWS, viewDirWS, light, 1, albedo, roughness, metallic);
}

vec3 LitWithLights(vec3 normalWS, vec3 positionWS, vec3 albedo, float roughness, float metallic)
{
    vec3 viewDirWS = normalize(GetCameraPositionWS() - positionWS);

    vec3 result = vec3(0);

    result += LitWithParallelLights(normalWS, positionWS, viewDirWS, albedo, roughness, metallic);
    result += LitWithPointLights(normalWS, positionWS, viewDirWS, albedo, roughness, metallic);
    result += LitWithIndirectLights(normalWS, viewDirWS, albedo, roughness, metallic);

    return result;
}
