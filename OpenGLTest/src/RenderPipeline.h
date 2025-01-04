#pragma once

#include "Entity.h"
#include "Material.h"
#include "Scene.h"

class RenderContext
{
public:
    glm::mat4 vMatrix;
    glm::mat4 pMatrix;
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
    static RenderPipeline* instance;
    
    size_t mainLightShadowTexSize = 4096;
    
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void setScreenSize(int width, int height);
    void render(RESOURCE_ID cameraId, const Scene* scene);
    void getViewProjMatrix(glm::mat4& view, glm::mat4& proj);
    void getScreenSize(int& width, int& height);

private:
    size_t m_screenWidth;
    size_t m_screenHeight;
    GLFWwindow* m_window;
    RenderContext m_renderContext;

    RESOURCE_ID m_skyboxCubeTexture = UNDEFINED_RESOURCE;
    RESOURCE_ID m_lutTexture = UNDEFINED_RESOURCE;
    
    RESOURCE_ID m_sphereMesh = UNDEFINED_RESOURCE;

    RESOURCE_ID m_skyboxMat = UNDEFINED_RESOURCE;
    RESOURCE_ID m_drawShadowMat = UNDEFINED_RESOURCE;
    RESOURCE_ID m_finalBlitMat = UNDEFINED_RESOURCE;

    RESOURCE_ID m_gBufferRenderTarget = UNDEFINED_RESOURCE;
    RESOURCE_ID m_shadingRenderTarget = UNDEFINED_RESOURCE;
    RESOURCE_ID m_mainLightShadowRenderTarget = UNDEFINED_RESOURCE;

    RESOURCE_ID m_fullScreenQuad = UNDEFINED_RESOURCE;
    RESOURCE_ID m_deferredShadingShader = UNDEFINED_RESOURCE;

    RESOURCE_ID m_gBuffer0Tex = UNDEFINED_RESOURCE;
    RESOURCE_ID m_gBuffer1Tex = UNDEFINED_RESOURCE;
    RESOURCE_ID m_gBuffer2Tex = UNDEFINED_RESOURCE;
    RESOURCE_ID m_gBufferDepthTex = UNDEFINED_RESOURCE;
    RESOURCE_ID m_shadingBufferTex = UNDEFINED_RESOURCE;
    RESOURCE_ID m_mainLightShadowMapTex = UNDEFINED_RESOURCE;

    bool _updateRenderTargetsPass();
    void _preparingPass(RESOURCE_ID cameraId);
    void _renderMainLightShadowPass(unsigned long long cameraId, const Scene* scene);
    void _renderSkyboxPass(RESOURCE_ID cameraId);
    void _renderScenePass(RESOURCE_ID cameraId, const Scene* scene);
    void _deferredShadingPass();
    void _finalBlitPass();
    void _renderUiPass();

    void _renderScene(const Scene* scene);
    void _renderEntity(const Entity* entity);
    void _renderMesh(const ::Mesh* mesh, ::Material* mat, const glm::mat4& m);
    void _setViewProjMatrix(RESOURCE_ID camera);
    void _setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj);
};
