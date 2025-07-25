#include "deferred_shading_pass.h"

#include "mesh.h"
#include "rendering_utils.h"
#include "material.h"
#include "render_texture.h"
#include "render_target.h"

namespace op
{
    DeferredShadingPass::DeferredShadingPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        m_quadMesh = Mesh::LoadFromFile("meshes/quad.obj");
        INCREF(m_quadMesh);
        m_deferredShadingMat = Material::CreateEmptyMaterial("shaders/deferred_shading.glsl");
        INCREF(m_deferredShadingMat);
    }

    DeferredShadingPass::~DeferredShadingPass()
    {
        DECREF(m_quadMesh);
        DECREF(m_deferredShadingMat);
    
        if (m_shadingRt)
        {
            m_renderContext->UnRegisterRt(m_shadingRt);
            DECREF(m_shadingRt);
            m_renderContext->UnRegisterRt(m_tempPpRt0);
            DECREF(m_tempPpRt0);
            m_renderContext->UnRegisterRt(m_tempPpRt1);
            DECREF(m_tempPpRt1);
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

        RenderingUtils::RenderMesh(*m_renderContext, m_quadMesh, m_deferredShadingMat, Matrix4x4::Identity());
    }

    void DeferredShadingPass::UpdateRt()
    {
        if (m_shadingRt == nullptr)
        {
            auto desc = RenderTextureDescriptor(
                m_renderContext->screenWidth,
                m_renderContext->screenHeight,
                RGBAHdr,
                Bilinear,
                Clamp,
                "_ShadingBufferTex");
            m_shadingRt = new RenderTexture(desc);
            INCREF(m_shadingRt);
            m_renderContext->RegisterRt(m_shadingRt);
            Material::SetGlobalTextureValue("_ShadingBufferTex", m_shadingRt);

            desc.name = "_TempPpRt0";
            m_tempPpRt0 = new RenderTexture(desc);
            INCREF(m_tempPpRt0);
            m_renderContext->RegisterRt(m_tempPpRt0);
            desc.name = "_TempPpRt1";
        
            m_tempPpRt1 = new RenderTexture(desc);
            INCREF(m_tempPpRt1);
            m_renderContext->RegisterRt(m_tempPpRt1);
        }
        m_shadingRt->Resize(m_renderContext->screenWidth, m_renderContext->screenHeight);
        m_tempPpRt0->Resize(m_renderContext->screenWidth, m_renderContext->screenHeight);
        m_tempPpRt1->Resize(m_renderContext->screenWidth, m_renderContext->screenHeight);
    }
}
