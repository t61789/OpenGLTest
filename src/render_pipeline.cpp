#include "render_pipeline.h"

#include <tracy/Tracy.hpp>

#include "culling_system.h"
#include "render_texture.h"
#include "gui.h"

#include "utils.h"
#include "scene.h"
#include "indirect_lighting.h"
#include "scene_object_indices.h"
#include "objects/light_comp.h"
#include "objects/camera_comp.h"
#include "objects/render_comp.h"
#include "render/render_target.h"
#include "render/texture_set.h"
#include "render/gl/gl_state.h"
#include "render/gl/gl_texture.h"
#include "render_pass/batch_render_pass.h"
#include "render_pass/deferred_shading_pass.h"
#include "render_pass/final_blit_pass.h"
#include "render_pass/kawase_blur.h"
#include "render_pass/dual_kawase_blur.h"
#include "render_pass/main_light_shadow_pass.h"
#include "render_pass/preparing_pass.h"
#include "render_pass/render_scene_pass.h"
#include "render_pass/render_skybox_pass.h"
#include "render_pass/test_draw_pass.h"

namespace op
{
    RenderPipeline::RenderPipeline(const uint32_t width, const uint32_t height, GLFWwindow* window)
    {
        m_window = window;
        SetScreenSize(width, height);

        m_renderTargetPool = mup<RenderTargetPool>();
        m_renderContext = mup<RenderContext>();

        m_gBuffer0Tex = msp<RenderTexture>(RtDesc{"_GBuffer0Tex", width, height, TextureFormat::RGBA_HDR, TextureFilterMode::POINT, TextureWrapMode::CLAMP});
        m_gBuffer1Tex = msp<RenderTexture>(RtDesc{"_GBuffer1Tex", width, height, TextureFormat::RGBA_HDR, TextureFilterMode::POINT, TextureWrapMode::CLAMP});
        m_gBuffer2Tex = msp<RenderTexture>(RtDesc{"_GBuffer2Tex", width, height, TextureFormat::DEPTH_TEX, TextureFilterMode::POINT, TextureWrapMode::CLAMP});
        m_gBufferDepthTex = msp<RenderTexture>(RtDesc{"_GBufferDepthTex", width, height, TextureFormat::DEPTH_STENCIL, TextureFilterMode::POINT, TextureWrapMode::CLAMP});

        GetGlobalTextureSet()->SetTexture(GBUFFER_0_TEX, m_gBuffer0Tex);
        GetGlobalTextureSet()->SetTexture(GBUFFER_1_TEX, m_gBuffer1Tex);
        GetGlobalTextureSet()->SetTexture(GBUFFER_2_TEX, m_gBuffer2Tex);
        
        m_passes.push_back(msp<PreparingPass>());
        m_passes.push_back(msp<RenderSkyboxPass>());
        m_passes.push_back(msp<MainLightShadowPass>());
        m_passes.push_back(msp<BatchRenderPass>());
        m_passes.push_back(msp<RenderScenePass>());
        // m_passes.push_back(msp<TestDrawPass>());
        m_passes.push_back(msp<DeferredShadingPass>());
        // m_passes.push_back(msp<KawaseBlur>());
        // m_passes.push_back(msp<DualKawaseBlur>());
        m_passes.push_back(msp<FinalBlitPass>());
    }

    RenderPipeline::~RenderPipeline()
    {
        m_passes.clear();
    }

    void RenderPipeline::SetScreenSize(const uint32_t width, const uint32_t height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    void RenderPipeline::Render(const CameraComp* camera, Scene* scene)
    {
        ZoneScoped;
        
        if(!UpdateRenderTargetsPass())
        {
            THROW_ERROR("RenderTarget创建失败")
        }

        PrepareRenderContext(scene);
        
        for (const auto& pass : m_passes)
        {
            begin_debug_group(pass->GetName().c_str());
            pass->Execute();
            end_debug_group();
        }
        
        begin_debug_group("Draw UI");
        RenderUiPass(m_renderContext.get());
        end_debug_group();

        GlState::Ins()->Reset();
        //
        // GL_CHECK_ERROR(帧绘制结束)

        SwapBuffers();
    }

    void RenderPipeline::GetScreenSize(uint32_t& width, uint32_t& height)
    {
        width = this->m_screenWidth;
        height = this->m_screenHeight;
    }

    void RenderPipeline::PrepareRenderContext(Scene* scene)
    {
        ZoneScoped;

        m_renderContext->camera = CameraComp::GetMainCamera(); // TODO 依赖于scene
        m_renderContext->scene = scene;
        m_renderContext->renderTargetPool = m_renderTargetPool.get();
        m_renderContext->mainLightShadowSize = mainLightShadowTexSize;
        m_renderContext->screenWidth = m_screenWidth;
        m_renderContext->screenHeight = m_screenHeight;
        m_renderContext->gBufferTextures = vecwp<RenderTexture>{
            m_gBuffer0Tex,
            m_gBuffer1Tex,
            m_gBuffer2Tex,
            m_gBufferDepthTex
        };

        CategorizeObjects(*m_renderContext);
    }

    bool RenderPipeline::UpdateRenderTargetsPass()
    {
        m_gBuffer0Tex->Resize(m_screenWidth, m_screenHeight);
        m_gBuffer1Tex->Resize(m_screenWidth, m_screenHeight);
        m_gBuffer2Tex->Resize(m_screenWidth, m_screenHeight);
        m_gBufferDepthTex->Resize(m_screenWidth, m_screenHeight);
        
        return true;
    }

    void RenderPipeline::RenderUiPass(const RenderContext* context)
    {
        ZoneScoped;

        Gui::Ins()->Render(context);
    }

    void RenderPipeline::CategorizeObjects(RenderContext& renderContext)
    {
        ZoneScoped;
        
        // TODO 每一帧都遍历整个场景开销不菲，可以在物体退出和进入场景的时候进行更新
        auto objectIndices = renderContext.scene->GetIndices();
        renderContext.allSceneObjs = &objectIndices->GetAllObjects();
        renderContext.allRenderObjs = &objectIndices->GetCompStorage()->GetComps<RenderComp>();
        renderContext.lights = &objectIndices->GetCompStorage()->GetComps<LightComp>();
        renderContext.cameras = &objectIndices->GetCompStorage()->GetComps<CameraComp>();
    }
    
    void RenderPipeline::SwapBuffers()
    {
        ZoneScoped;
        
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}
