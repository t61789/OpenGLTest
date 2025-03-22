uniform float _Shc[27];

void CalcBaseShc(vec3 direction, out float baseShc[9])
{
    baseShc[0] = 0.282095f;
    baseShc[1] = 0.488603f * direction.y;
    baseShc[2] = 0.488603f * direction.z;
    baseShc[3] = 0.488603f * direction.x;
    baseShc[4] = 1.092548f * direction.y * direction.x;
    baseShc[5] = 1.092548f * direction.y * direction.z;
    baseShc[6] = 0.315392f * (-direction.x * direction.x - direction.y * direction.y + 2 * direction.z * direction.z);
    baseShc[7] = 1.092548f * direction.z * direction.x;
    baseShc[8] = 0.546274f * (direction.x * direction.x - direction.y * direction.y);
}

vec3 CalcIndirectLighting(vec3 direction)
{
    direction = normalize(direction);
    
    float baseShc[9];
    CalcBaseShc(direction, baseShc);
    
    vec3 result = vec3(0.0f);
    for (int i = 0; i < 9; i++)
    {
        result.r += baseShc[i] * _Shc[i * 3 + 0];
        result.g += baseShc[i] * _Shc[i * 3 + 1];
        result.b += baseShc[i] * _Shc[i * 3 + 2];
    }
    
    return result;
}
