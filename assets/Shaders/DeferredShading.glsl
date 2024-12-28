#version 330 core

layout (location = 0) in vec3 aPositionOS;
layout (location = 1) in vec3 aNormalOS;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aColor;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPositionOS.xy, 0, 1);
    texCoord = aTexcoord;
}

#version 330 core

#include "./Common.glsl"

in vec2 texCoord;

uniform float _ExposureMultiplier;

out vec4 FragColor;

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

vec3 GGXSpecular(float roughness, vec3 normalWS, vec3 viewDir, vec3 F0)
{
    vec3 n = normalWS;
    vec3 v = viewDir;
    vec3 l = _MainLightDirection.xyz;
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

vec3 Lit(vec3 albedo, vec3 normalWS, float roughness, float metallic)
{
    vec3 positionWS = TransformScreenToWorld(texCoord);
    float mainLightShadowAttenuation = SampleShadowMap(positionWS);
    vec3 mainLightColor = _MainLightColor.rgb * mainLightShadowAttenuation;
    vec3 viewDir = normalize(GetCameraPositionWS() - positionWS);
    vec3 ambient = _AmbientLightColor.rgb;

    vec3 l = _MainLightDirection.xyz;
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
    
    vec3 diffuse = albedo * ndl * mainLightColor;
    vec3 H = normalize(_MainLightDirection.xyz + viewDir);
    vec3 specular = GGXSpecular(roughness, normalWS, viewDir, F0) * mainLightColor;
    
    vec3 environmentReflection = SampleSkybox(reflect(-v, n)) * Luminance(_MainLightColor.rgb);
    vec3 eF = F0 + (1.0 - F0) * pow(max(1.0 - ndv, 0), 5.0);
    environmentReflection *= eF; // TODO 预计算
    
    return kD * diffuse + specular;
}

vec3 LitSimple(vec3 albedo, vec3 normalWS, float roughness, float metallic)
{
    vec3 positionWS = TransformScreenToWorld(texCoord);

    float mainLightShadowAttenuation = SampleShadowMap(positionWS);

    vec3 mainLightColor = _MainLightColor.rgb * mainLightShadowAttenuation;

    vec3 viewDir = normalize(GetCameraPositionWS() - positionWS);

    vec3 ambient = _AmbientLightColor.rgb;
    vec3 diffuse = mainLightColor * albedo * max(dot(normalWS, _MainLightDirection.xyz), 0);
    vec3 H = normalize(_MainLightDirection.xyz + viewDir);
    vec3 specular = mainLightColor * albedo * pow(max(dot(normalWS, H), 0), mix(20, 0.01, roughness)) * 5;
    
    diffuse *= 1 - metallic;
    specular *= metallic;

//    albedo = SampleSkybox(normalWS);
//    return SampleSkybox(reflect(-viewDir, normalWS));

    return diffuse + specular + ambient;
}

void main()
{
    vec3 albedo;
    int pixelType;
    vec3 normalWS;
    ReadGBuffer0(texCoord, albedo, pixelType);
    ReadGBuffer1(texCoord, normalWS);
    
    vec3 finalColor;
    if(pixelType == PIXEL_TYPE_LIT)
    {
        finalColor = Lit(albedo, normalWS, 0.2, 0.0);
    }
    else
    {
        finalColor = albedo;
    }

    FragColor = vec4(finalColor, 1);
//    FragColor = vec4(albedo, 1);
//    FragColor = vec4(mainLightShadowAttenuation);
//    FragColor = vec4(mod(positionWS.xz, 1), 0, 1);
}
