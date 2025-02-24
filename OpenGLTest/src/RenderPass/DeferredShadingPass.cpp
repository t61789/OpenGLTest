#include "DeferredShadingPass.h"

#include "RenderingUtils.h"

DeferredShadingPass::DeferredShadingPass(RenderContext* renderContext) : RenderPass(renderContext)
{
    m_quadMesh = Mesh::LoadFromFile("meshes/quad.obj");
    m_quadMesh->IncRef();
    m_deferredShadingMat = Material::CreateEmptyMaterial("shaders/deferred_shading.glsl");
    m_deferredShadingMat->IncRef();
}

DeferredShadingPass::~DeferredShadingPass()
{
    m_quadMesh->DecRef();
    m_deferredShadingMat->DecRef();
    
    if (m_shadingRt)
    {
        m_renderContext->UnRegisterRt(m_shadingRt);
        m_shadingRt->DecRef();
        m_renderContext->UnRegisterRt(m_tempPpRt0);
        m_tempPpRt0->DecRef();
        m_renderContext->UnRegisterRt(m_tempPpRt1);
        m_tempPpRt1->DecRef();
    }
}

std::string DeferredShadingPass::GetName()
{
    return "Deferred Shading Pass";
}

void DeferredShadingPass::Execute()
{
    if(m_quadMesh == nullptr || m_deferredShadingMat == nullptr)
    {
        return;
    }

    UpdateRt();
    
    RenderTarget::Get(m_shadingRt, nullptr)->Use();

    RenderingUtils::RenderMesh(*m_renderContext, m_quadMesh, m_deferredShadingMat, glm::mat4(1));
}

void DeferredShadingPass::UpdateRt()
{
    if (m_shadingRt == nullptr)
    {
        auto desc = RenderTextureDescriptor(
            m_renderContext->screenWidth,
            m_renderContext->screenHeight,
            RGBAHdr,
            Point,
            Clamp,
            "_ShadingBufferTex");
        m_shadingRt = new RenderTexture(desc);
        m_shadingRt->IncRef();
        m_renderContext->RegisterRt(m_shadingRt);
        Material::SetGlobalTextureValue("_ShadingBufferTex", m_shadingRt);

        desc.name = "_TempPpRt0";
        m_tempPpRt0 = new RenderTexture(desc);
        m_tempPpRt0->IncRef();
        m_renderContext->RegisterRt(m_tempPpRt0);
        desc.name = "_TempPpRt1";
        
        m_tempPpRt1 = new RenderTexture(desc);
        m_tempPpRt1->IncRef();
        m_renderContext->RegisterRt(m_tempPpRt1);
    }
    m_shadingRt->Resize(m_renderContext->screenWidth, m_renderContext->screenHeight);
    m_tempPpRt0->Resize(m_renderContext->screenWidth, m_renderContext->screenHeight);
    m_tempPpRt1->Resize(m_renderContext->screenWidth, m_renderContext->screenHeight);
}

