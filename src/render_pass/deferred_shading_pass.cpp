#include "deferred_shading_pass.h"

#include <tracy/Tracy.hpp>

#include "built_in_res.h"
#include "mesh.h"
#include "rendering_utils.h"

#include "render_texture.h"
#include "render_target.h"

namespace op
{
    DeferredShadingPass::DeferredShadingPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        auto shader = Shader::LoadFromFile("shaders/deferred_shading.shader");
        m_deferredShadingMat = new Material();
        m_deferredShadingMat->BindShader(shader);
        INCREF(m_deferredShadingMat);
    }

    DeferredShadingPass::~DeferredShadingPass()
    {
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
        ZoneScoped;
        
        if(m_deferredShadingMat == nullptr)
        {
            return;
        }

        UpdateRt();
    
        RenderTarget::Get(m_shadingRt, nullptr)->Use();

        auto quadMesh = BuiltInRes::Ins()->quadMesh;
        RenderingUtils::RenderMesh(quadMesh, m_deferredShadingMat, Matrix4x4::Identity(), Matrix4x4::Identity());
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
            GameResource::Ins()->GetPredefinedMaterial(GLOBAL_CBUFFER)->Set(SHADING_BUFFER_TEX, m_shadingRt);

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
