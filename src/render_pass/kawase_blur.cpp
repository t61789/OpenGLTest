// #include "kawase_blur.h"
//
// #include <tracy/Tracy.hpp>
//
// #include "imgui.h"
//
// #include "gui.h"
// #include "render_texture.h"
//
// #include "rendering_utils.h"
//
// namespace op
// {
//     KawaseBlur::KawaseBlur(RenderContext* renderContext) : IRenderPass(renderContext)
//     {
//         rt = new RenderTexture(RtDesc(
//             2,
//             2,
//             RenderTextureFormat::RGBA,
//             BILINEAR,
//             CLAMP));
//         INCREF(rt)
//
//         // m_kawaseBlitMat = Material::CreateEmptyMaterial("shaders/kawase_blit.glsl");
//         // INCREF(m_kawaseBlitMat) TODO
//     }
//
//     KawaseBlur::~KawaseBlur()
//     {
//         DECREF(rt)
//         // DECREF(m_kawaseBlitMat)
//     }
//
//     std::string KawaseBlur::GetName()
//     {
//         return "Kawase Blur";
//     }
//
//     void KawaseBlur::Execute()
//     {
//         ZoneScoped;
//         
//         auto shadingRt = m_renderContext->GetRt("_ShadingBufferTex");
//         auto rt0 = shadingRt;
//         auto rt1 = m_renderContext->GetRt("_TempPpRt0");
//
//         if (!rt0 || !rt1)
//         {
//             return;
//         }
//
//         for (int i = 0; i < m_iteration; ++i)
//         {
//             // m_kawaseBlitMat->SetTextureValue(MAIN_TEX, rt0);
//             // m_kawaseBlitMat->SetFloatValue(ITERATIONS, static_cast<float>(i)); // TODO
//
//             // RenderingUtils::Blit(rt0, rt1, m_kawaseBlitMat); // TODO
//
//             auto temp = rt0;
//             rt0 = rt1;
//             rt1 = temp;
//         }
//
//         if (m_iteration % 2)
//         {
//             // RenderingUtils::Blit(rt0, shadingRt, nullptr); // TODO
//         }
//     }
//
//     void KawaseBlur::DrawConsoleUi()
//     {
//         ImGui::SliderInt("Kawase Blur Iterations", &m_iteration, 0, 16);
//     }
// }
