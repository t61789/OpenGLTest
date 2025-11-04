#include "deferred_shading_pass.h"

#include <tracy/Tracy.hpp>

#include "material.h"
#include "mesh.h"
#include "rendering_utils.h"
#include "render_context.h"

#include "render_texture.h"
#include "shader.h"
#include "render/gl/gl_state.h"
#include "render/gl/gl_texture.h"

namespace op
{
    DeferredShadingPass::DeferredShadingPass()
    {
        auto shader = Shader::LoadFromFile("shaders/deferred_shading.shader");
        m_deferredShadingMat = msp<Material>();
        m_deferredShadingMat->BindShader(shader);
        m_deferredShadingMat->depthMode = DepthMode::ALWAYS;
        m_deferredShadingMat->depthWrite = false;
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

        RenderingUtils::Blit(nullptr, m_shadingRt, m_deferredShadingMat.get());
    }

    void DeferredShadingPass::UpdateRt()
    {
        if (m_shadingRt == nullptr)
        {
            RtDesc desc =
            {
                "_ShadingBufferTex",
                GetRC()->screenWidth,
                GetRC()->screenHeight,
                TextureFormat::RGBA,
                TextureFilterMode::BILINEAR,
                TextureWrapMode::CLAMP,
            };
            m_shadingRt = msp<RenderTexture>(desc);
            GetRC()->shadingBufferTex = m_shadingRt;
        }
        
        m_shadingRt->Resize(GetRC()->screenWidth, GetRC()->screenHeight);
    }
}
