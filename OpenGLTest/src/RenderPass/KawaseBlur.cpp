#include "KawaseBlur.h"

#include "RenderingUtils.h"

KawaseBlur::KawaseBlur()
{
    rt = new RenderTexture(RenderTextureDescriptor(
        2,
        2,
        RenderTextureFormat::RGBA,
        Bilinear,
        Clamp));
    rt->IncRef();

    kawaseBlitMat = Material::CreateEmptyMaterial("shaders/kawase_blit.glsl");
    kawaseBlitMat->IncRef();
}

KawaseBlur::~KawaseBlur()
{
    rt->DecRef();
    kawaseBlitMat->DecRef();
}

std::string KawaseBlur::GetName()
{
    return "Kawase Blur";
}

void KawaseBlur::Execute(RenderContext& renderContext)
{
    auto rt0 = renderContext.shadingRt;
    auto rt1 = renderContext.tempPpRt0;

    if (!rt0 || !rt1)
    {
        return;
    }

    for (int i = 0; i < renderContext.kawaseBlurIterations; ++i)
    {
        kawaseBlitMat->SetTextureValue("_MainTex", rt0);
        kawaseBlitMat->SetFloatValue("_Iterations", static_cast<float>(i));

        RenderingUtils::Blit(rt0, rt1, kawaseBlitMat);

        auto temp = rt0;
        rt0 = rt1;
        rt1 = temp;
    }

    if (renderContext.kawaseBlurIterations % 2)
    {
        RenderingUtils::Blit(rt0, renderContext.shadingRt, nullptr);
    }
}
