#pragma once

#include <unordered_map>

#include "string_handle.h"

#define PI 3.1415926535f
#define DEG2RAD 0.0174532925f
#define RAD2DEG 57.2957795f
#define EPSILON 1e-6

#define SHADER_PARAM(KEY, VALUE) static auto KEY = StringHandle(#VALUE)

SHADER_PARAM(CAMERA_POSITION_WS, _CameraPositionWS);
SHADER_PARAM(MIN_LUMINANCE, _MinLuminance);
SHADER_PARAM(MAX_LUMINANCE, _MaxLuminance);
SHADER_PARAM(LUT_TEX, _LutTex);
SHADER_PARAM(VIEWPORT_SIZE, _ViewportSize);
SHADER_PARAM(FOG_INTENSITY, _FogIntensity);
SHADER_PARAM(FOG_COLOR, _FogColor);
SHADER_PARAM(MAIN_LIGHT_DIRECTION, _MainLightDirection);
SHADER_PARAM(PARALLEL_LIGHT_INFO, _ParallelLightInfo);
SHADER_PARAM(PARALLEL_LIGHT_COUNT, _ParallelLightCount);
SHADER_PARAM(POINT_LIGHT_INFO, _PointLightInfo);
SHADER_PARAM(POINT_LIGHT_COUNT, _PointLightCount);
SHADER_PARAM(VP, _VP);
SHADER_PARAM(IVP, _IVP);
SHADER_PARAM(MVP, _MVP);
SHADER_PARAM(ITM, _ITM);
SHADER_PARAM(M, _M);
SHADER_PARAM(IM, _IM);
SHADER_PARAM(BLUR_SIZE, _BlurSize);
SHADER_PARAM(APPLY_THRESHOLD, _ApplyThreshold);
SHADER_PARAM(THRESHOLD, _THRESHOLD);
SHADER_PARAM(SHC, _Shc);
SHADER_PARAM(MAIN_TEX, _MainTex);
SHADER_PARAM(EXPOSURE_MULTIPLIER, _ExposureMultiplier);
SHADER_PARAM(MAINLIGHT_SHADOW_VP, _MainLightShadowVP);
SHADER_PARAM(ITERATIONS, _Iterations);

#define VERTEX_ATTRIB_NUM_NEW 1
extern const char* VERTEX_ATTRIB_NAMES_NEW[VERTEX_ATTRIB_NUM_NEW];
extern const int VERTEX_ATTRIB_FLOAT_COUNT_NEW[VERTEX_ATTRIB_NUM_NEW];

enum class VertexAttr : std::uint8_t
{
    POSITION_OS,
    NORMAL_OS,
    TANGENT_OS,
    UV0,
    UV1,
    COUNT
};

inline std::unordered_map<VertexAttr, uint32_t> VERTEX_ATTR_STRIDE =
{
    {VertexAttr::POSITION_OS, 4},
    {VertexAttr::NORMAL_OS, 4},
    {VertexAttr::TANGENT_OS, 4},
    {VertexAttr::UV0, 2},
    {VertexAttr::UV1, 2},
};

inline std::unordered_map<VertexAttr, std::string> VERTEX_ATTR_NAME =
{
    {VertexAttr::POSITION_OS, "positionOS"},
    {VertexAttr::NORMAL_OS, "normalOS"},
    {VertexAttr::TANGENT_OS, "tangentOS"},
    {VertexAttr::UV0, "uv0"},
    {VertexAttr::UV1, "uv1"},
};
