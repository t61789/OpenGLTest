#include "RenderPipeline.h"

#include <queue>
#include <stack>
#include <glm.hpp>

#include "Camera.h"
#include "Entity.h"
#include "GameFramework.h"
#include "Gui.h"
#include "Image.h"
#include "RenderTarget.h"
#include "RenderTexture.h"
#include "RenderPass/DeferredShadingPass.h"
#include "RenderPass/FinalBlitPass.h"
#include "RenderPass/MainLightShadowPass.h"
#include "RenderPass/PreparingPass.h"
#include "RenderPass/RenderSkyboxPass.h"

RenderPipeline* RenderPipeline::instance = nullptr;

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    instance = this;

    m_window = window;
    SetScreenSize(width, height);

    m_passes.push_back(new PreparingPass());
    m_passes.push_back(new MainLightShadowPass());
    m_passes.push_back(new RenderSkyboxPass());
    m_passes.push_back(new DeferredShadingPass());
    m_passes.push_back(new FinalBlitPass());

    m_cullModeMgr = std::make_unique<CullModeMgr>();

    m_sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
    m_sphereMesh->IncRef();

    m_quadMesh = Mesh::LoadFromFile("meshes/quad.obj");
    m_quadMesh->IncRef();

    m_gBuffer0Tex = new RenderTexture(width, height, RGBAHdr, Point, Clamp, "_GBuffer0Tex");
    m_gBuffer0Tex->IncRef();
    m_gBuffer1Tex = new RenderTexture(width, height, RGBA, Point, Clamp, "_GBuffer1Tex");
    m_gBuffer1Tex->IncRef();
    m_gBuffer2Tex = new RenderTexture(width, height, DepthTex, Point, Clamp, "_GBuffer2Tex");
    m_gBuffer2Tex->IncRef();
    m_gBufferDepthTex = new RenderTexture(width, height, DepthStencil, Point, Clamp, "_GBufferDepthTex");
    m_gBufferDepthTex->IncRef();
    Material::SetGlobalTextureValue("_GBuffer0Tex", m_gBuffer0Tex);
    Material::SetGlobalTextureValue("_GBuffer1Tex", m_gBuffer1Tex);
    Material::SetGlobalTextureValue("_GBuffer2Tex", m_gBuffer2Tex);

    m_gBufferDesc = std::make_unique<RenderTargetDesc>();
    m_gBufferDesc->SetColorAttachment(0, m_gBuffer0Tex);
    m_gBufferDesc->SetColorAttachment(1, m_gBuffer1Tex);
    m_gBufferDesc->SetColorAttachment(2, m_gBuffer2Tex);
    m_gBufferDesc->SetDepthAttachment(m_gBufferDepthTex, true);
}

RenderPipeline::~RenderPipeline()
{
    instance = nullptr;

    for (auto pass : m_passes)
    {
        delete pass;
    }
    m_passes.clear();

    m_sphereMesh->DecRef();
    m_quadMesh->DecRef();
        
    m_gBuffer0Tex->DecRef();
    m_gBuffer1Tex->DecRef();
    m_gBuffer2Tex->DecRef();
    m_gBufferDepthTex->DecRef();
}

void RenderPipeline::SetScreenSize(const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void RenderPipeline::Render(const Camera* camera, Scene* scene)
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

    auto renderContext = PrepareRenderContext(scene);
    for (auto pass : m_passes)
    {
        Utils::BeginDebugGroup(pass->GetName());
        pass->Execute(renderContext);
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
    view = m_renderContext.vMatrix;
    proj = m_renderContext.pMatrix;
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

RenderContext RenderPipeline::PrepareRenderContext(Scene* scene)
{
    auto renderContext = RenderContext();
    renderContext.camera = Camera::GetMainCamera(); // TODO 依赖于scene
    renderContext.scene = scene;
    renderContext.cullModeMgr = m_cullModeMgr.get();
    renderContext.mainLightShadowSize = mainLightShadowTexSize;
    renderContext.screenWidth = m_screenWidth;
    renderContext.screenHeight = m_screenHeight;
    renderContext.gBufferDesc = m_gBufferDesc.get();
    renderContext.quadMesh = m_quadMesh;
    return renderContext;
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
