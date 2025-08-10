#include "render_pipeline.h"

#include <tracy/Tracy.hpp>

#include "render_texture.h"
#include "gui.h"
#include "culling_system.h"
#include "cull_mode.h"
#include "blend_mode.h"
#include "built_in_res.h"
#include "render_target.h"
#include "material.h"
#include "utils.h"
#include "scene.h"
#include "indirect_lighting.h"
#include "objects/light_comp.h"
#include "objects/camera_comp.h"
#include "render_pass/deferred_shading_pass.h"
#include "render_pass/final_blit_pass.h"
#include "render_pass/kawase_blur.h"
#include "render_pass/dual_kawase_blur.h"
#include "render_pass/main_light_shadow_pass.h"
#include "render_pass/preparing_pass.h"
#include "render_pass/render_scene_pass.h"
#include "render_pass/render_skybox_pass.h"

namespace op
{
    RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
    {
        m_window = window;
        SetScreenSize(width, height);

        m_renderContext = std::make_unique<RenderContext>();
        m_cullModeMgr = std::make_unique<CullModeMgr>();
        m_blendModeMgr = std::make_unique<BlendModeMgr>();
        m_textureBindingMgr = std::make_unique<TextureBindingMgr>();
        m_cullingSystem = std::make_unique<CullingSystem>(m_renderContext.get());

        m_gBuffer0Tex = new RenderTexture(width, height, RGBAHdr, Point, Clamp, "_GBuffer0Tex");
        INCREF(m_gBuffer0Tex);
        m_renderContext->RegisterRt(m_gBuffer0Tex);
        m_gBuffer1Tex = new RenderTexture(width, height, RGBAHdr, Point, Clamp, "_GBuffer1Tex");
        INCREF(m_gBuffer1Tex);
        m_renderContext->RegisterRt(m_gBuffer1Tex);
        m_gBuffer2Tex = new RenderTexture(width, height, DepthTex, Point, Clamp, "_GBuffer2Tex");
        INCREF(m_gBuffer2Tex);
        m_renderContext->RegisterRt(m_gBuffer2Tex);
        m_gBufferDepthTex = new RenderTexture(width, height, DepthStencil, Point, Clamp, "_GBufferDepthTex");
        INCREF(m_gBufferDepthTex);
        m_renderContext->RegisterRt(m_gBufferDepthTex);
        Material::SetGlobalTextureValue("_GBuffer0Tex", m_gBuffer0Tex);
        Material::SetGlobalTextureValue("_GBuffer1Tex", m_gBuffer1Tex);
        Material::SetGlobalTextureValue("_GBuffer2Tex", m_gBuffer2Tex);
        
        m_gBufferDesc = std::make_unique<RenderTargetDesc>();
        m_gBufferDesc->SetColorAttachment(0, m_gBuffer0Tex);
        m_gBufferDesc->SetColorAttachment(1, m_gBuffer1Tex);
        m_gBufferDesc->SetColorAttachment(2, m_gBuffer2Tex);
        m_gBufferDesc->SetDepthAttachment(m_gBufferDepthTex, true);

        m_passes.push_back(new PreparingPass(m_renderContext.get())); // TODO 改成shared_ptr
        m_passes.push_back(new MainLightShadowPass(m_renderContext.get()));
        m_passes.push_back(new RenderSkyboxPass(m_renderContext.get()));
        m_passes.push_back(new RenderScenePass(m_renderContext.get()));
        m_passes.push_back(new DeferredShadingPass(m_renderContext.get()));
        // m_passes.push_back(new KawaseBlur());
        // m_passes.push_back(new DualKawaseBlur(m_renderContext.get()));
        m_passes.push_back(new FinalBlitPass(m_renderContext.get()));
    }

    RenderPipeline::~RenderPipeline()
    {
        for (auto pass : m_passes)
        {
            delete pass;
        }
        m_passes.clear();

        DECREF(m_gBuffer0Tex);
        DECREF(m_gBuffer1Tex);
        DECREF(m_gBuffer2Tex);
        DECREF(m_gBufferDepthTex);

        RenderTarget::ClearAllCache();
        Material::ClearAllGlobalValues();
    }

    void RenderPipeline::SetScreenSize(const int width, const int height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    void RenderPipeline::Render(const CameraComp* camera, Scene* scene)
    {
        ZoneScoped;
        
        if(!UpdateRenderTargetsPass())
        {
            throw std::runtime_error("RenderTarget创建失败");
        }

        if (m_preDrawnScene != scene)
        {
            m_preDrawnScene = scene;
            FirstDrawScene(scene);
        }

        PrepareRenderContext(scene);
        
        for (auto pass : m_passes)
        {
            Utils::BeginDebugGroup(pass->GetName());
            pass->Execute();
            Utils::EndDebugGroup();
        }
        
        Utils::BeginDebugGroup("Draw UI");
        RenderUiPass(m_renderContext.get());
        Utils::EndDebugGroup();

        SwapBuffers();
        
        Utils::ClearGlError();
    }

    void RenderPipeline::GetViewProjMatrix(Matrix4x4& view, Matrix4x4& proj)
    {
        view = m_renderContext->vMatrix;
        proj = m_renderContext->pMatrix;
    }

    void RenderPipeline::GetScreenSize(int& width, int& height)
    {
        width = this->m_screenWidth;
        height = this->m_screenHeight;
    }

    void RenderPipeline::FirstDrawScene(const Scene* scene)
    {
        
    }

    void RenderPipeline::PrepareRenderContext(Scene* scene)
    {
        ZoneScoped;
        
        m_renderContext->camera = CameraComp::GetMainCamera(); // TODO 依赖于scene
        m_renderContext->scene = scene;
        m_renderContext->cullModeMgr = m_cullModeMgr.get();
        m_renderContext->blendModeMgr = m_blendModeMgr.get();
        m_renderContext->textureBindingMgr = m_textureBindingMgr.get();
        m_renderContext->mainLightShadowSize = mainLightShadowTexSize;
        m_renderContext->screenWidth = m_screenWidth;
        m_renderContext->screenHeight = m_screenHeight;
        m_renderContext->gBufferDesc = m_gBufferDesc.get();

        CategorizeObjects(*m_renderContext);
        
        m_cullingSystem->Cull();
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

        BuiltInRes::GetInstance()->quadMesh;
        
        
        Gui::GetInstance()->Render(context);
    }

    void RenderPipeline::CategorizeObjects(RenderContext& renderContext)
    {
        ZoneScoped;
        
        // TODO 每一帧都遍历整个场景开销不菲，可以在物体退出和进入场景的时候进行更新
        auto objectIndices = renderContext.scene->objectIndices.get();
        renderContext.allSceneObjs = objectIndices->GetAllObjects();
        renderContext.allRenderObjs = objectIndices->GetAllComps<RenderComp>("RenderComp");
        renderContext.lights = objectIndices->GetAllComps<LightComp>("LightComp");
        renderContext.cameras = objectIndices->GetAllComps<CameraComp>("CameraComp");
    }
    void RenderPipeline::SwapBuffers()
    {
        ZoneScoped;
        
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}
