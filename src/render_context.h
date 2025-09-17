#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "utils.h"
#include "math/math.h"

namespace op
{
    struct UsingRenderTarget;
    class RenderTargetPool;
    class RenderTarget;
    class RenderTexture;
    class Scene;
    class Object;
    class CameraComp;
    class RenderComp;
    class LightComp;

    class RenderContext : public Singleton<RenderContext>
    {
    public:
        uint32_t screenWidth = 0;
        uint32_t screenHeight = 0;

        int mainLightShadowSize = 0;
        
        Matrix4x4 vMatrix;
        Matrix4x4 pMatrix;
        Matrix4x4 vpMatrix;
        Vec3 cameraPositionWS;

        LightComp* mainLight = nullptr;

        vecwp<RenderTexture> gBufferTextures;
        wp<RenderTexture> shadingBufferTex;
        
        CameraComp* camera = nullptr;
        Scene* scene = nullptr;
        RenderTargetPool* renderTargetPool = nullptr;

        const vecwp<Object>* allSceneObjs;
        
        const vecwp<LightComp>* lights;
        const vecwp<CameraComp>* cameras;
        const vecwp<RenderComp>* allRenderObjs;
        vec<RenderComp*> visibleRenderObjs;

        RenderContext() = default;
        ~RenderContext() = default;
        RenderContext(const RenderContext& other) = delete;
        RenderContext(RenderContext&& other) noexcept = delete;
        RenderContext& operator=(const RenderContext& other) = delete;
        RenderContext& operator=(RenderContext&& other) noexcept = delete;

        void SetViewProjMatrix(const CameraComp* cam);
        void SetViewProjMatrix(const Matrix4x4& view, const Matrix4x4& proj);
        void SetViewProjMatrix(const Matrix4x4& view, const Matrix4x4& proj, const Vec3& cameraPos);
        UsingRenderTarget UsingGBufferRenderTarget();

    private:
        std::unordered_map<std::string, RenderTexture*> m_rts;
    };

    static RenderContext* GetRC()
    {
        return RenderContext::Ins();
    }
}

