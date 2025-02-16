#pragma once

#include "Camera.h"
#include "CullMode.h"
#include "Entity.h"
#include "IndirectLighting.h"
#include "Material.h"
#include "RenderTarget.h"
#include "RenderTexture.h"
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
    Material* replaceMaterial = nullptr;
};

class RenderPipeline
{
public:
    static RenderPipeline* instance;

    int mainLightShadowTexSize = 4096;
    
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void SetScreenSize(int width, int height);
    void Render(const Camera* camera, Scene* scene);
    void GetViewProjMatrix(glm::mat4& view, glm::mat4& proj);
    void GetScreenSize(int& width, int& height);

private:
    int m_screenWidth;
    int m_screenHeight;
    GLFWwindow* m_window = nullptr;
    CullModeMgr* m_cullModeMgr = nullptr;
    RenderContext m_renderContext;

    Scene* m_preDrawnScene = nullptr;

    Texture* m_skyboxCubeTexture = nullptr;
    Texture* m_lutTexture = nullptr;
    
    Mesh* m_sphereMesh = nullptr;
    Mesh* m_quadMesh = nullptr;

    Material* m_skyboxMat = nullptr;
    Material* m_drawShadowMat = nullptr;
    Material* m_deferredShadingMat = nullptr;
    Material* m_finalBlitMat = nullptr;

    RenderTarget* m_gBufferRenderTarget = nullptr;
    RenderTarget* m_shadingRenderTarget = nullptr;
    RenderTarget* m_mainLightShadowRenderTarget = nullptr;

    RenderTexture* m_gBuffer0Tex = nullptr;
    RenderTexture* m_gBuffer1Tex = nullptr;
    RenderTexture* m_gBuffer2Tex = nullptr;
    RenderTexture* m_gBufferDepthTex = nullptr;
    RenderTexture* m_shadingBufferTex = nullptr;
    RenderTexture* m_mainLightShadowMapTex = nullptr;

    void FirstDrawScene(const Scene* scene);

    bool UpdateRenderTargetsPass();
    void PreparingPass(const Camera* camera);
    void RenderMainLightShadowPass(const Camera* camera, const Scene* scene);
    void RenderSkyboxPass(const Camera* camera);
    void RenderScenePass(const Camera* camera, const Scene* scene);
    void DeferredShadingPass();
    void FinalBlitPass();
    void RenderUiPass();

    void RenderScene(const Scene* scene);
    void RenderEntity(const Entity* entity);
    void RenderMesh(const ::Mesh* mesh, ::Material* mat, const glm::mat4& m);
    void SetViewProjMatrix(const Camera* camera);
    void SetViewProjMatrix(const glm::mat4& view, const glm::mat4& proj);
};
