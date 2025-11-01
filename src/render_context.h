#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "utils.h"
#include "math/math.h"
#include "render/render_target.h"

namespace op
{
    class RenderTargetPool;
    class RenderTarget;
    class RenderTexture;
    class Scene;
    class Object;
    class CameraComp;
    class RenderComp;
    class LightComp;


    struct ViewProjInfo
    {
        Matrix4x4 vMatrix;
        Matrix4x4 pMatrix;
        Matrix4x4 vpMatrix;
        Vec3 viewCenter;
        std::optional<std::array<Vec4, 6>> frustumPlanes = std::nullopt;
        std::optional<Matrix4x4> ivpMatrix;

        void UpdateFrustumPlanes();
        void UpdateIVP();

        static sp<ViewProjInfo> Create(cr<Matrix4x4> vMatrix, cr<Matrix4x4> pMatrix, bool useIVP = false);
        static sp<ViewProjInfo> Create(cr<Matrix4x4> vMatrix, cr<Matrix4x4> pMatrix, cr<Vec3> viewCenter, bool useIVP = false);
    };
    

    class RenderContext : public Singleton<RenderContext>
    {
        struct VPMatrix
        {
            Matrix4x4 vMatrix;
            Matrix4x4 pMatrix;
            Vec3 cameraPos;
        };
        
    public:
        uint32_t screenWidth = 0;
        uint32_t screenHeight = 0;

        uint32_t mainLightShadowSize = 0;
        
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

        sp<ViewProjInfo> mainVPInfo = nullptr;
        sp<ViewProjInfo> shadowVPInfo = nullptr;

        RenderContext() = default;
        ~RenderContext() = default;
        RenderContext(const RenderContext& other) = delete;
        RenderContext(RenderContext&& other) noexcept = delete;
        RenderContext& operator=(const RenderContext& other) = delete;
        RenderContext& operator=(RenderContext&& other) noexcept = delete;

        UsingRenderTargetObj UsingGBufferRenderTarget();
        
        void PushViewProjMatrix(crsp<ViewProjInfo> viewProjInfo);
        void PopViewProjMatrix();
        UsingObject UsingViewProjMatrix(crsp<ViewProjInfo> viewProjInfo);
        crsp<ViewProjInfo> CurViewProjMatrix() const;

    private:
        std::unordered_map<std::string, RenderTexture*> m_rts;
        vecsp<ViewProjInfo> m_vpMatrixStack;

        void SetViewProjMatrix(crsp<ViewProjInfo> viewProjInfo);
    };

    static RenderContext* GetRC()
    {
        return RenderContext::Ins();
    }
}

