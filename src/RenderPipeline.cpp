#include "RenderPipeline.h"

#include "Gui.h"
#include "CullingSystem.h"
#include "CullMode.h"
#include "RenderTexture.h"
#include "RenderTarget.h"
#include "Material.h"
#include "Utils.h"
#include "Scene.h"
#include "IndirectLighting.h"
#include "Objects/LightComp.h"
#include "Objects/CameraComp.h"
#include "RenderPass/DeferredShadingPass.h"
#include "RenderPass/FinalBlitPass.h"
#include "RenderPass/KawaseBlur.h"
#include "RenderPass/DualKawaseBlur.h"
#include "RenderPass/MainLightShadowPass.h"
#include "RenderPass/PreparingPass.h"
#include "RenderPass/RenderScenePass.h"
#include "RenderPass/RenderSkyboxPass.h"

#include "glfw3.h"

RenderPipeline* RenderPipeline::instance = nullptr;

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    instance = this;

    m_window = window;
    SetScreenSize(width, height);

    m_renderContext = std::make_unique<RenderContext>();
    m_cullModeMgr = std::make_unique<CullModeMgr>();
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

    m_passes.push_back(new PreparingPass(m_renderContext.get()));
    m_passes.push_back(new MainLightShadowPass(m_renderContext.get()));
    m_passes.push_back(new RenderSkyboxPass(m_renderContext.get()));
    m_passes.push_back(new RenderScenePass(m_renderContext.get()));
    m_passes.push_back(new DeferredShadingPass(m_renderContext.get()));
    // m_passes.push_back(new KawaseBlur());
    m_passes.push_back(new DualKawaseBlur(m_renderContext.get()));
    m_passes.push_back(new FinalBlitPass(m_renderContext.get()));
}

RenderPipeline::~RenderPipeline()
{
    instance = nullptr;

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
    RenderUiPass();
    Utils::EndDebugGroup();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
    
    Utils::ClearGlError();
}

void RenderPipeline::GetViewProjMatrix(glm::mat4& view, glm::mat4& proj)
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
    IndirectLighting::SetGradientAmbientColor(scene->ambientLightColorSky, scene->ambientLightColorEquator, scene->ambientLightColorGround);
    
}

void RenderPipeline::PrepareRenderContext(Scene* scene)
{
    m_renderContext->camera = CameraComp::GetMainCamera(); // TODO 依赖于scene
    m_renderContext->scene = scene;
    m_renderContext->cullModeMgr = m_cullModeMgr.get();
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

void RenderPipeline::RenderUiPass()
{
    Gui::Render();
}

void RenderPipeline::CategorizeObjects(RenderContext& renderContext)
{
    // TODO 每一帧都遍历整个场景开销不菲，可以在物体退出和进入场景的时候进行更新
    renderContext.scene->GetAllObjects(renderContext.allSceneObjs);
    renderContext.allRenderObjs.clear();
    renderContext.lights.clear();
    renderContext.cameras.clear();
    
    for (auto obj : renderContext.allSceneObjs)
    {
        auto renderComp = obj->GetComp<RenderComp>("RenderComp");
        if (renderComp)
        {
            renderContext.allRenderObjs.push_back(renderComp);
        }

        auto lightComp = obj->GetComp<LightComp>("LightComp");
        if (lightComp)
        {
            renderContext.lights.push_back(lightComp);
        }

        auto cameraComp = obj->GetComp<CameraComp>("CameraComp");
        if (cameraComp)
        {
            renderContext.cameras.push_back(cameraComp);
        }
    }
}
