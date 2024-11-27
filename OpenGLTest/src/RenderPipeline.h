#pragma once

#include "Entity.h"
#include "Material.h"
#include "Scene.h"

class RenderContext
{
public:
    glm::mat4 vpMatrix;
    glm::vec3 cameraPositionWS;
    glm::vec3 mainLightDirection;
    glm::vec3 mainLightColor;
    glm::vec3 ambientLightColor;
    float tonemappingExposureMultiplier = 1;
    RESOURCE_ID replaceMaterial = UNDEFINED_RESOURCE;
};

class RenderPipeline
{
public:
    size_t mainLightShadowTexSize = 4096;
    
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void setScreenSize(int width, int height);
    void render(RESOURCE_ID cameraId, const Scene* scene);

private:
    size_t m_screenWidth;
    size_t m_screenHeight;
    GLFWwindow* m_window;

    RESOURCE_ID m_drawShadowMat = UNDEFINED_RESOURCE;

    RESOURCE_ID m_gBufferRenderTarget = UNDEFINED_RESOURCE;
    RESOURCE_ID m_shadingRenderTarget = UNDEFINED_RESOURCE;
    RESOURCE_ID m_mainLightShadowRenderTarget = UNDEFINED_RESOURCE;

    RESOURCE_ID m_fullScreenQuad = UNDEFINED_RESOURCE;
    RESOURCE_ID m_deferredShadingShader = UNDEFINED_RESOURCE;
    RESOURCE_ID m_finalBlitShader = UNDEFINED_RESOURCE;

    bool _updateRenderTargetsPass();
    void _clearRenderTargetsPass();
    void _drawMainLightShadowPass(unsigned long long cameraId, const Scene* scene, RenderContext& renderContext);
    void _renderScenePass(RESOURCE_ID cameraId, const Scene* scene, RenderContext& renderContext);
    void _deferredShadingPass();
    void _finalBlitPass();

    void _renderScene(const Scene* scene, const RenderContext& renderContext);
    void _renderEntity(const Entity* entity, const RenderContext& renderContext);
    void _setViewProjMatrix(RESOURCE_ID camera, RenderContext& renderContext);
    void _setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj, RenderContext& renderContext);
};
