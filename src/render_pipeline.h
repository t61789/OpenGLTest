#pragma once

#include <memory>
#include <vector>

#include "render_context.h"
#include "render/render_target_pool.h"
#include "utils.h"

namespace op
{
    struct Matrix4x4;
    class RenderTarget;
    class Object;
    class CameraComp;
    class Scene;
    class RenderTargetDesc;
    class RenderTexture;
    class IRenderPass;

    class RenderPipeline : public Singleton<RenderPipeline>
    {
    public:
        int mainLightShadowTexSize = 4096;

        RenderPipeline(uint32_t width, uint32_t height, GLFWwindow* window);
        ~RenderPipeline();
        RenderPipeline(const RenderPipeline& other) = delete;
        RenderPipeline(RenderPipeline&& other) noexcept = delete;
        RenderPipeline& operator=(const RenderPipeline& other) = delete;
        RenderPipeline& operator=(RenderPipeline&& other) noexcept = delete;

        void SetScreenSize(uint32_t width, uint32_t height);
        void GetScreenSize(uint32_t& width, uint32_t& height);
        void Render(const CameraComp* camera, Scene* scene);
        void GetViewProjMatrix(Matrix4x4& view, Matrix4x4& proj);

    private:
        uint32_t m_screenWidth;
        uint32_t m_screenHeight;

        GLFWwindow* m_window = nullptr;
        up<RenderContext> m_renderContext = nullptr;
        up<RenderTargetPool> m_renderTargetPool = nullptr;

        sp<RenderTexture> m_gBuffer0Tex = nullptr;
        sp<RenderTexture> m_gBuffer1Tex = nullptr;
        sp<RenderTexture> m_gBuffer2Tex = nullptr;
        sp<RenderTexture> m_gBufferDepthTex = nullptr;

        vecsp<IRenderPass> m_passes;
        
        void PrepareRenderContext(Scene* scene);
        bool UpdateRenderTargetsPass();
        void RenderUiPass(const RenderContext* renderContext);
        void SwapBuffers();
        
        static void CategorizeObjects(RenderContext& renderContext);
    };
}
