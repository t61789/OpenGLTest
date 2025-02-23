#pragma once

#include <glm.hpp>

#include "Camera.h"
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

    RenderTexture* shadingRt = nullptr;
    RenderTexture* tempPpRt0 = nullptr;

    void SetViewProjMatrix(const Camera* cam);
    void SetViewProjMatrix(const glm::mat4& view, const glm::mat4& proj);
};

