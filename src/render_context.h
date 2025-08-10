#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "math/math.h"
#include "render/texture_binding_mgr.h"

namespace op
{
    class CullModeMgr;
    class BlendModeMgr;
    class Material;
    class RenderTargetDesc;
    class RenderTexture;
    class Scene;
    class Object;
    class CameraComp;
    class RenderComp;
    class LightComp;

    class RenderContext
    {
    public:
        int screenWidth = 0;
        int screenHeight = 0;

        int mainLightShadowSize = 0;
        
        Matrix4x4 vMatrix;
        Matrix4x4 pMatrix;
        Matrix4x4 vpMatrix;
        Vec3 cameraPositionWS;
        Material* replaceMaterial = nullptr;

        LightComp* mainLight = nullptr;

        RenderTargetDesc* gBufferDesc = nullptr;
        
        CameraComp* camera = nullptr;
        Scene* scene = nullptr;
        CullModeMgr* cullModeMgr = nullptr;
        BlendModeMgr* blendModeMgr = nullptr;
        TextureBindingMgr* textureBindingMgr = nullptr;

        std::vector<Object*>* allSceneObjs;
        
        std::vector<LightComp*>* lights;
        std::vector<CameraComp*>* cameras;
        std::vector<RenderComp*>* allRenderObjs;
        std::vector<RenderComp*> visibleRenderObjs;

        ~RenderContext();

        void SetViewProjMatrix(const CameraComp* cam);
        void SetViewProjMatrix(const Matrix4x4& view, const Matrix4x4& proj);

        void RegisterRt(RenderTexture* rt);
        void UnRegisterRt(const RenderTexture* rt);
        RenderTexture* GetRt(const std::string& name);

    private:
        std::unordered_map<std::string, RenderTexture*> m_rts;
    };
}

