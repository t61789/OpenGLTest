#pragma once

#include <glm/glm.hpp>

#include "Camera.h"
#include "Entity.h"
#include "Material.h"
#include "RenderTarget.h"
#include "Scene.h"

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

    RenderTargetDesc* gBufferDesc = nullptr;
    
    Camera* camera = nullptr;
    Scene* scene = nullptr;
    CullModeMgr* cullModeMgr = nullptr;

    std::vector<Object*>* allSceneObjs = nullptr;
    std::vector<Entity*>* allRenderObjs = nullptr;
    std::vector<Entity*>* visibleRenderObjs = nullptr;

    ~RenderContext();

    void SetViewProjMatrix(const Camera* cam);
    void SetViewProjMatrix(const glm::mat4& view, const glm::mat4& proj);

    void RegisterRt(RenderTexture* rt);
    void UnRegisterRt(const RenderTexture* rt);
    RenderTexture* GetRt(const std::string& name);

private:
    std::unordered_map<std::string, RenderTexture*> m_rts;
};

