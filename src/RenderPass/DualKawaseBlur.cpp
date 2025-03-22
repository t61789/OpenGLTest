#include "DualKawaseBlur.h"

#include "imgui.h"

#include "RenderTexture.h"
#include "Material.h"
#include "RenderingUtils.h"

DualKawaseBlur::DualKawaseBlur(RenderContext* renderContext) : RenderPass(renderContext)
{
    m_downsampleMat = Material::CreateEmptyMaterial("shaders/dual_kawase_downsample.glsl");
    INCREF(m_downsampleMat);
    m_upsampleMat = Material::CreateEmptyMaterial("shaders/dual_kawase_upsample.glsl");
    INCREF(m_upsampleMat);
}

DualKawaseBlur::~DualKawaseBlur()
{
    ReleaseRt();

    DECREF(m_downsampleMat);
    DECREF(m_upsampleMat);
}

std::string DualKawaseBlur::GetName()
{
    return "Dual Kawase Blur";
}

void DualKawaseBlur::Execute()
{
    auto shadingRt = m_renderContext->GetRt("_ShadingBufferTex");
    if (!shadingRt || m_maxIterations == 0)
    {
        ReleaseRt();
        return;
    }

    UpdateRt(shadingRt);

    m_downsampleMat->SetFloatValue("_BlurSize", static_cast<float>(m_blurSize));
    m_upsampleMat->SetFloatValue("_BlurSize", static_cast<float>(m_blurSize));

    for (int i = 0; i < m_blurTextures.size(); ++i)
    {
        RenderTexture* from;
        RenderTexture* to = m_blurTextures[i];
        if (i == 0)
        {
            from = shadingRt;
            m_downsampleMat->SetFloatValue("_ApplyThreshold", 1);
            m_downsampleMat->SetFloatValue("_Threshold", m_threshold);
        }
        else
        {
            from = m_blurTextures[i - 1];
            m_downsampleMat->SetFloatValue("_ApplyThreshold", 0);
        }

        RenderingUtils::Blit(from, to, m_downsampleMat);
    }

    for (int i = static_cast<int>(m_blurTextures.size()) - 1; i > 0; --i)
    {
        auto from = m_blurTextures[i];
        auto to = m_blurTextures[i - 1];
        
        RenderingUtils::Blit(from, to, m_upsampleMat);
        
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_ONE, GL_ONE);
        // RenderingUtils::Blit(m_blurTextures[i - 1], shadingRt);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glDisable(GL_BLEND);
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    RenderingUtils::Blit(m_blurTextures.front(), shadingRt);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
}

void DualKawaseBlur::DrawConsoleUi()
{
    if (!ImGui::CollapsingHeader("Dual Kawase Blur"))
    {
        return;
    }
    
    ImGui::SliderInt("Max Iterations", &m_maxIterations, 0, 10);
    ImGui::SliderInt("Blur Size", &m_blurSize, 1, 10);
    ImGui::SliderFloat("Threshold", &m_threshold, 0, 4);
}

void DualKawaseBlur::UpdateRt(const RenderTexture* shadingRt)
{
    auto curWidth = shadingRt->width >> 1;
    auto curHeight = shadingRt->height >> 1;
    auto iteration = GetIteration(shadingRt->width, shadingRt->height);
    
    if (!m_blurTextures.empty() &&
        m_blurTextures.front()->width == curWidth &&
        m_blurTextures.front()->height == curHeight &&
        m_blurTextures.size() == iteration)
    {
        return;
    }

    ReleaseRt();
    for (int i = 0; i < iteration; ++i)
    {
        auto desc = shadingRt->desc;
        desc.width = curWidth;
        desc.height = curHeight;
        m_blurTextures.push_back(new RenderTexture(desc));
        INCREF(m_blurTextures.back());

        curWidth >>= 1;
        curHeight >>= 1;
    }
}

void DualKawaseBlur::ReleaseRt()
{
    for (auto& rt : m_blurTextures)
    {
        DECREF(rt);
    }

    m_blurTextures.clear();
}

int DualKawaseBlur::GetIteration(const int originWidth, const int originHeight)
{
    auto curWidth = originWidth;
    auto curHeight = originHeight;

    auto iteration = 0;
    while (curWidth != 0 && curHeight != 0)
    {
        iteration++;

        curWidth >>= 1;
        curHeight >>= 1;
    }
    iteration -= 1;
    
    iteration = std::min(iteration, m_maxIterations);

    return iteration;
}
