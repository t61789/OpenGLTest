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

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<CullModeMgr> m_cullModeMgr = nullptr;
    std::unique_ptr<RenderTargetDesc> m_gBufferDesc = nullptr;
    std::unique_ptr<RenderContext> m_renderContext = nullptr;

    Scene* m_preDrawnScene = nullptr;

    RenderTexture* m_gBuffer0Tex = nullptr;
    RenderTexture* m_gBuffer1Tex = nullptr;
    RenderTexture* m_gBuffer2Tex = nullptr;
    RenderTexture* m_gBufferDepthTex = nullptr;

    std::function<void()>* m_guiCallBack = nullptr;

    std::vector<RenderPass*> m_passes;
    
    void FirstDrawScene(const Scene* scene);
    void PrepareRenderContext(Scene* scene);
    bool UpdateRenderTargetsPass();
    void RenderUiPass();
    void OnGuiConsole();
};
