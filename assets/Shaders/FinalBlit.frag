#version 330 core

in vec2 texCoord;

uniform sampler2D _MainTex;

uniform float _ExposureMultiplier;

out vec4 FragColor;

vec3 applyToneMapping(vec3 inputColor)
{
    const mat3 preTonemappingTransform = mat3(
        0.575961650, 0.070806820, 0.028035252,
        0.344143820, 0.827392350, 0.131523770,
        0.079952030, 0.101774690, 0.840242300
    );

    const mat3 postTonemappingTransform = mat3(
        1.666954300, -0.106835220, -0.004142626,
        -0.601741150,  1.237778600, -0.087411870,
        -0.065202855, -0.130948950,  1.091555000
    );

//    mat3 exposedPreTonemappingTransform = _ExposureMultiplier * preTonemappingTransform;
//
//    vec3 color = vec3(exposedPreTonemappingTransform * inputColor);
    vec3 color = inputColor;

    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0, 1);

//    color = vec3(postTonemappingTransform * color);
    
    return color;
}

void main()
{
    vec4 color = texture(_MainTex, texCoord);

    color.rgb = applyToneMapping(color.rgb);
    
    FragColor = vec4(color.rgb, 1);
};