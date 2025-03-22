#pragma once

#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"

#include "CullMode.h"

namespace op
{
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
        
        glm::mat4 vMatrix;
        glm::mat4 pMatrix;
        glm::mat4 vpMatrix;
        glm::vec3 cameraPositionWS;
        Material* replaceMaterial = nullptr;

        LightComp* mainLight = nullptr;

        RenderTargetDesc* gBufferDesc = nullptr;
        
        CameraComp* camera = nullptr;
        Scene* scene = nullptr;
        CullModeMgr* cullModeMgr = nullptr;

        std::vector<Object*> allSceneObjs;
        
        std::vector<LightComp*> lights;
        std::vector<CameraComp*> cameras;
        std::vector<RenderComp*> allRenderObjs;
        std::vector<RenderComp*> visibleRenderObjs;

        ~RenderContext();

        void SetViewProjMatrix(const CameraComp* cam);
        void SetViewProjMatrix(const glm::mat4& view, const glm::mat4& proj);

        void RegisterRt(RenderTexture* rt);
        void UnRegisterRt(const RenderTexture* rt);
        RenderTexture* GetRt(const std::string& name);

    private:
        std::unordered_map<std::string, RenderTexture*> m_rts;
    };
}

