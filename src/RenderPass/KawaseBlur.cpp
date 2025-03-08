#include "KawaseBlur.h"

#include "imgui.h"

#include "Gui.h"
#include "RenderTexture.h"
#include "Material.h"
#include "RenderingUtils.h"

KawaseBlur::KawaseBlur(RenderContext* renderContext) : RenderPass(renderContext)
{
    rt = new RenderTexture(RenderTextureDescriptor(
        2,
        2,
        RenderTextureFormat::RGBA,
        Bilinear,
        Clamp));
    INCREF(rt);

    m_kawaseBlitMat = Material::CreateEmptyMaterial("shaders/kawase_blit.glsl");
    INCREF(m_kawaseBlitMat);

    m_guiConsoleCallBack = new std::function<void()>([this]{this->OnGuiConsole();});
    Gui::drawConsoleEvent.AddCallBack(m_guiConsoleCallBack);
}

KawaseBlur::~KawaseBlur()
{
    DECREF(rt);
    DECREF(m_kawaseBlitMat);

    Gui::drawConsoleEvent.RemoveCallBack(m_guiConsoleCallBack);
    delete m_guiConsoleCallBack;
}

std::string KawaseBlur::GetName()
{
    return "Kawase Blur";
}

void KawaseBlur::Execute()
{
    auto shadingRt = m_renderContext->GetRt("_ShadingBufferTex");
    auto rt0 = shadingRt;
    auto rt1 = m_renderContext->GetRt("_TempPpRt0");

    if (!rt0 || !rt1)
    {
        return;
    }

    for (int i = 0; i < m_iteration; ++i)
    {
        m_kawaseBlitMat->SetTextureValue("_MainTex", rt0);
        m_kawaseBlitMat->SetFloatValue("_Iterations", static_cast<float>(i));

        RenderingUtils::Blit(rt0, rt1, m_kawaseBlitMat);

        auto temp = rt0;
        rt0 = rt1;
        rt1 = temp;
    }

    if (m_iteration % 2)
    {
        RenderingUtils::Blit(rt0, shadingRt, nullptr);
    }
}

void KawaseBlur::OnGuiConsole()
{
    ImGui::SliderInt("Kawase Blur Iterations", &m_iteration, 0, 16);
}
