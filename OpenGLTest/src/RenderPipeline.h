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
};

class RenderPipeline
{
public:
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void setScreenSize(int width, int height);
    void render(RESOURCE_ID cameraId, const Scene* scene);

private:
    size_t m_screenWidth;
    size_t m_screenHeight;
    GLFWwindow* m_window;

    RESOURCE_ID m_gBufferRenderTarget;
    RESOURCE_ID m_shadingRenderTarget;

    RESOURCE_ID m_fullScreenQuad = UNDEFINED_RESOURCE;
    RESOURCE_ID m_deferredShadingShader = UNDEFINED_RESOURCE;
    RESOURCE_ID m_finalBlitShader = UNDEFINED_RESOURCE;

    bool _updateRenderTargetsPass();
    void _clearRenderTargetsPass();
    void _renderScenePass(RESOURCE_ID cameraId, const Scene* scene, RenderContext& renderContext);
    void _renderEntity(const Entity* entity, const RenderContext& renderContext);
    void _deferredShadingPass();
    void _finalBlitPass();
};
