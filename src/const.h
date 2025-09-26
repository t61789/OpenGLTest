#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <boost/lockfree/lockfree_forward.hpp>

#include "string_handle.h"
#include "common/simple_list.h"

namespace op
{
    #define TRACY_IDLE_COLOR 0x25281E
    
    static bool LOG_RENDER_TARGET_STACK_NOT_EMPTY = false;
    static bool ENABLE_GL_CHECK_ERROR = false;
    
    #define THROW_ERROR(msg) throw std::runtime_error(format_log(Error, msg));
    #define THROW_ERRORF(msg, ...) throw std::runtime_error(format_log(Error, msg, __VA_ARGS__));

    template <typename T>
    using up = std::unique_ptr<T>;
    template <typename T>
    using wp = std::weak_ptr<T>;
    template <typename T>
    using sp = std::shared_ptr<T>;
    template <typename T>
    using cr = const T&;
    template <typename T>
    
    using vec = std::vector<T>;
    template <typename T>
    using vecsp = std::vector<sp<T>>;
    template <typename T>
    using vecwp = std::vector<wp<T>>;
    template <typename T>
    using vecpt = std::vector<T*>;
    template <typename T>
    using cvec = const vec<T>;
    template <typename T>
    using crvec = cr<vec<T>>;
    template <typename T>
    using crvecsp = cr<vecsp<T>>;
    template <typename T>
    using crvecwp = cr<vecwp<T>>;
    template <typename T>
    using crvecpt = cr<vecpt<T>>;

    template <typename T>
    using sl = SimpleList<T>;
    template <typename T>
    using crsl = cr<sl<T>>;
    
    template <typename K, typename V>
    using umap = std::unordered_map<K, V>;
    template <typename K, typename V>
    using crumap = cr<umap<K, V>>;
    template <typename T>
    using crsp = const std::shared_ptr<T>&;
    template <typename T>
    using crwp = cr<wp<T>>;
    template <typename T>
    using cpt = const T*;
    template <typename T>
    using c = const T;
    using cstr = const char*;
    template <typename T, size_t N>
    using arr = std::array<T, N>;
    using str = std::string;
    using crstr = cr<std::string>;
    template <typename T>
    using lock_free_queue = boost::lockfree::queue<T, boost::lockfree::fixed_sized<true>>;
    
    template <typename T, typename... Args>
    std::unique_ptr<T> mup(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    template <typename T, typename... Args>
    std::shared_ptr<T> msp(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
    
    #define PI 3.1415926535f
    #define DEG2RAD 0.0174532925f
    #define RAD2DEG 57.2957795f
    #define EPSILON 1e-6
    #define TEXTURE_SLOT_LIMIT 12
    
    #define STRING_HANDLE(KEY, VALUE) static auto KEY = StringHandle(#VALUE);

    STRING_HANDLE(GLOBAL_CBUFFER, _Global)
    STRING_HANDLE(PER_VIEW_CBUFFER, PerViewCBuffer)
    STRING_HANDLE(PER_MATERIAL_CBUFFER, PerMaterialCBuffer)
    STRING_HANDLE(OBJECT_INDEX_CBUFFER, ObjectIndexCBuffer)

    STRING_HANDLE(MIN_LUMINANCE, _MinLuminance)
    STRING_HANDLE(MAX_LUMINANCE, _MaxLuminance)
    STRING_HANDLE(LUT_TEX, _LutTex)
    STRING_HANDLE(VIEWPORT_SIZE, _ViewportSize)
    STRING_HANDLE(FOG_INTENSITY, _FogIntensity)
    STRING_HANDLE(FOG_COLOR, _FogColor)
    STRING_HANDLE(MAIN_LIGHT_DIRECTION, _MainLightDirection)
    STRING_HANDLE(PARALLEL_LIGHT_INFO, _ParallelLightInfo)
    STRING_HANDLE(PARALLEL_LIGHT_COUNT, _ParallelLightCount)
    STRING_HANDLE(POINT_LIGHT_INFO, _PointLightInfo)
    STRING_HANDLE(POINT_LIGHT_COUNT, _PointLightCount)
    STRING_HANDLE(VP, _VP)
    STRING_HANDLE(IVP, _IVP)
    STRING_HANDLE(MVP, _MVP)
    STRING_HANDLE(ITM, _ITM)
    STRING_HANDLE(M, _M)
    STRING_HANDLE(IM, _IM)
    STRING_HANDLE(BLUR_SIZE, _BlurSize)
    STRING_HANDLE(APPLY_THRESHOLD, _ApplyThreshold)
    STRING_HANDLE(THRESHOLD, _THRESHOLD)
    STRING_HANDLE(SHC, _Shc)
    STRING_HANDLE(MAIN_TEX, _MainTex)
    STRING_HANDLE(GBUFFER_0_TEX, _GBuffer0Tex)
    STRING_HANDLE(GBUFFER_1_TEX, _GBuffer1Tex)
    STRING_HANDLE(GBUFFER_2_TEX, _GBuffer2Tex)
    STRING_HANDLE(SHADING_BUFFER_TEX, _ShadingBufferTex)
    STRING_HANDLE(SKYBOX_TEX, _SkyboxTex)
    STRING_HANDLE(MAIN_LIGHT_SHADOW_MAP_TEX, _MainLightShadowMapTex)
    STRING_HANDLE(EXPOSURE_MULTIPLIER, _ExposureMultiplier)
    STRING_HANDLE(MAINLIGHT_SHADOW_VP, _MainLightShadowVP)
    STRING_HANDLE(ITERATIONS, _Iterations)
    STRING_HANDLE(CAMERA_POSITION_WS, _CameraPositionWS)
    STRING_HANDLE(ALBEDO, _Albedo)
    STRING_HANDLE(ALBEDO_1, _Albedo1)
    STRING_HANDLE(OBJECT_INDEX, _ObjectIndex)

    STRING_HANDLE(UNNAMED_OBJECT, UnnamedObject)
    STRING_HANDLE(TRANSFORM_COMP, TransformComp)
    STRING_HANDLE(RUNTIME_COMP, RuntimeComp)
    STRING_HANDLE(RENDER_COMP, RenderComp)
    STRING_HANDLE(LIGHT_COMP, LightComp)
    STRING_HANDLE(CAMERA_COMP, CameraComp)

    inline std::string NOT_A_FILE = "NOT_A_FILE";

    static constexpr uint32_t MAX_SUPPORT_SLOTS = 12;
    static constexpr uint32_t PER_OBJECT_BUFFER_INIT_CAPACITY = 1000;
    static constexpr uint32_t PER_OBJECT_BUFFER_BINDING_SLOT = 0;

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

    struct VertexAttrDefine
    {
        VertexAttr attr;
        uint32_t index;
        uint32_t strideF;
        uint32_t offsetF;
        StringHandle name;
    };

    inline std::vector<VertexAttrDefine> VERTEX_ATTR_DEFINES =
    {
        {VertexAttr::POSITION_OS, static_cast<uint32_t>(VertexAttr::POSITION_OS), 4, 0, "positionOS"},
        {VertexAttr::NORMAL_OS, static_cast<uint32_t>(VertexAttr::NORMAL_OS), 4, 4, "normalOS"},
        {VertexAttr::TANGENT_OS, static_cast<uint32_t>(VertexAttr::TANGENT_OS), 4, 8, "tangentOS"},
        {VertexAttr::UV0, static_cast<uint32_t>(VertexAttr::UV0), 2, 12, "uv0"},
        {VertexAttr::UV1, static_cast<uint32_t>(VertexAttr::UV1), 2, 14, "uv1"},
    };
    
    static constexpr uint32_t MAX_VERTEX_ATTR_STRIDE_F = 16;

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

    inline std::unordered_set PREDEFINED_MATERIALS = {
        GLOBAL_CBUFFER.Hash(),
        PER_VIEW_CBUFFER.Hash(),
        OBJECT_INDEX_CBUFFER.Hash()
    };
}
