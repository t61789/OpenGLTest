#pragma once

#include "Camera.h"
#include "CullMode.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Scene.h"
#include "RenderPass/RenderPass.h"

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

    std::unique_ptr<RenderTargetDesc> m_gBufferDesc = nullptr;
    
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<CullModeMgr> m_cullModeMgr = nullptr;
    RenderContext m_renderContext;

    Scene* m_preDrawnScene = nullptr;

    Mesh* m_sphereMesh = nullptr;
    Mesh* m_quadMesh = nullptr;

    RenderTexture* m_gBuffer0Tex = nullptr;
    RenderTexture* m_gBuffer1Tex = nullptr;
    RenderTexture* m_gBuffer2Tex = nullptr;
    RenderTexture* m_gBufferDepthTex = nullptr;

    std::vector<RenderPass*> m_passes;

    void FirstDrawScene(const Scene* scene);

    RenderContext PrepareRenderContext(Scene* scene);
    bool UpdateRenderTargetsPass();
    void RenderUiPass();
};
