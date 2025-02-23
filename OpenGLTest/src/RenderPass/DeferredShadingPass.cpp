#include "DeferredShadingPass.h"

#include "RenderingUtils.h"

DeferredShadingPass::DeferredShadingPass()
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
        m_shadingRt->DecRef();
        m_tempPpRt0->DecRef();
    }
}

std::string DeferredShadingPass::GetName()
{
    return "Deferred Shading Pass";
}

void DeferredShadingPass::Execute(RenderContext& renderContext)
{
    if(m_quadMesh == nullptr || m_deferredShadingMat == nullptr)
    {
        return;
    }

    UpdateRt(renderContext);
    
    RenderTarget::Get(m_shadingRt, nullptr)->Use();

    RenderingUtils::RenderMesh(renderContext, m_quadMesh, m_deferredShadingMat, glm::mat4(1));
}

void DeferredShadingPass::UpdateRt(RenderContext& renderContext)
{
    if (m_shadingRt == nullptr)
    {
        auto desc = RenderTextureDescriptor(
            renderContext.screenWidth,
            renderContext.screenHeight,
            RGBAHdr,
            Point,
            Clamp,
            "_ShadingBufferTex");
        m_shadingRt = new RenderTexture(desc);
        m_shadingRt->IncRef();
        Material::SetGlobalTextureValue("_ShadingBufferTex", m_shadingRt);

        m_tempPpRt0 = new RenderTexture(desc);
        m_tempPpRt0->IncRef();
    }
    renderContext.shadingRt = m_shadingRt;
    renderContext.tempPpRt0 = m_tempPpRt0;
    m_shadingRt->Resize(renderContext.screenWidth, renderContext.screenHeight);
}

