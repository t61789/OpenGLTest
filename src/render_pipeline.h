#pragma once

#include <memory>
#include <vector>

#include "shader.h"
#include "utils.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "objects/render_comp.h"
#include "render/render_state.h"

namespace op
{
    class Object;
    class CameraComp;
    class Scene;
    class RenderTargetDesc;
    class RenderContext;
    class CullingSystem;
    class RenderTexture;
    class RenderPass;

    class RenderPipeline : public Singleton<RenderPipeline>
    {
    public:
        int mainLightShadowTexSize = 4096;

        RenderPipeline(int width, int height, GLFWwindow* window);
        ~RenderPipeline();
        void SetScreenSize(int width, int height);
        void Render(const CameraComp* camera, Scene* scene);
        void GetViewProjMatrix(Matrix4x4& view, Matrix4x4& proj);
        void GetScreenSize(int& width, int& height);

    private:
        int m_screenWidth;
        int m_screenHeight;

        GLFWwindow* m_window = nullptr;
        std::unique_ptr<TextureBindingMgr> m_textureBindingMgr = nullptr;
        std::unique_ptr<RenderTargetDesc> m_gBufferDesc = nullptr;
        std::unique_ptr<RenderContext> m_renderContext = nullptr;
        std::unique_ptr<CullingSystem> m_cullingSystem = nullptr;

        Scene* m_preDrawnScene = nullptr;
        std::unique_ptr<std::vector<Object*>> m_renderObjs = nullptr;

        RenderTexture* m_gBuffer0Tex = nullptr;
        RenderTexture* m_gBuffer1Tex = nullptr;
        RenderTexture* m_gBuffer2Tex = nullptr;
        RenderTexture* m_gBufferDepthTex = nullptr;

        std::vector<RenderPass*> m_passes;
        
        void FirstDrawScene(const Scene* scene);
        void PrepareRenderContext(Scene* scene);
        bool UpdateRenderTargetsPass();
        void RenderUiPass(const RenderContext* renderContext);
        void SwapBuffers();
        
        static void CategorizeObjects(RenderContext& renderContext);
    };
}
