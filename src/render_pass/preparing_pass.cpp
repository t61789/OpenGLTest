#include "preparing_pass.h"

#include "gui.h"
#include "indirect_lighting.h"
#include "render_target.h"
#include "material.h"
#include "scene.h"
#include "objects/camera_comp.h"
#include "objects/light_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    PreparingPass::PreparingPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
    }

    std::string PreparingPass::GetName()
    {
        return "Preparing Pass";
    }

    void PreparingPass::Execute()
    {
        if(m_renderContext->camera == nullptr)
        {
            return;
        }

        Material::SetGlobalVector4Value("_CameraPositionWS", Vec4(m_renderContext->camera->owner->transform->GetPosition(), 0));

        std::vector clearColors = {
            Vec4(0.5f),
            Vec4(0.0f),
            Vec4(1.0f),
        };
        RenderTarget::Get(*m_renderContext->gBufferDesc)->Clear(clearColors, 1.0f);

        auto viewportSize = Vec4(m_renderContext->screenWidth, m_renderContext->screenHeight, 0, 0);
        Material::SetGlobalVector4Value("_ViewportSize", viewportSize);

        PrepareLightInfos();
    
        IndirectLighting::SetGradientAmbientColor(
            m_renderContext->scene->ambientLightColorSky,
            m_renderContext->scene->ambientLightColorEquator,
            m_renderContext->scene->ambientLightColorGround);

        Material::SetGlobalFloatValue("_FogIntensity", m_renderContext->scene->fogIntensity);
        Material::SetGlobalVector4Value("_FogColor", Vec4(m_renderContext->scene->fogColor, 0));

        m_renderContext->SetViewProjMatrix(m_renderContext->camera);
    }

    void PreparingPass::DrawConsoleUi()
    {
        if (!m_renderContext->scene)
        {
            return;
        }

        if (!ImGui::CollapsingHeader("Ambient Lights"))
        {
            return;
        }
    
        m_renderContext->scene->ambientLightColorSky = Gui::SliderFloat3("GradientSky", m_renderContext->scene->ambientLightColorSky, 0.0f, 10.0f, "%.2f");
        m_renderContext->scene->ambientLightColorEquator = Gui::SliderFloat3("GradientEquator", m_renderContext->scene->ambientLightColorEquator, 0.0f, 10.0f, "%.2f");
        m_renderContext->scene->ambientLightColorGround = Gui::SliderFloat3("GradientGround", m_renderContext->scene->ambientLightColorGround, 0.0f, 10.0f, "%.2f");
        ImGui::SliderFloat("Fog Intensity", &m_renderContext->scene->fogIntensity, 0.0f, 0.005f, "%.5f");
        m_renderContext->scene->fogColor = Gui::SliderFloat3("Fog Color", m_renderContext->scene->fogColor, 0.0f, 1.0f, "%.2f");
    }

    void PreparingPass::PrepareLightInfos()
    {
        m_renderContext->mainLight = nullptr;

        auto parallelLights = std::vector<LightComp*>();
        auto pointLights = std::vector<LightComp*>();

        for (auto light : m_renderContext->lights)
        {
            constexpr int maxPointLights = 16;
            constexpr int maxParallelLights = 4;
            if (light->lightType == 0 && parallelLights.size() < maxParallelLights)
            {
                if (!m_renderContext->mainLight)
                {
                    m_renderContext->mainLight = light;
                    Material::SetGlobalVector4Value("_MainLightDirection", Vec4(-light->owner->transform->GetLocalToWorld().Forward(), 0));
                }

                parallelLights.push_back(light);
            }
            else if (light->lightType == 1 && pointLights.size() < maxPointLights)
            {
                pointLights.push_back(light);
            }
        }

        struct alignas(8) ParallelLightInfo
        {
            Vec4 param0; // x: dir.x, y: dir.y, z: dir.z, w: dummy
            Vec4 param1; // x: color.x, y: color.y, z: color.z, w: dummy
        };

        auto parallelLightInfos = std::vector<ParallelLightInfo>();
        parallelLightInfos.reserve(parallelLights.size());
        for (auto light : parallelLights)
        {
            parallelLightInfos.push_back({
                Vec4(-light->owner->transform->GetLocalToWorld().Forward(), 0),
                Vec4(light->GetColor(), 0)
            });
        }
        auto updateBuffer = reinterpret_cast<float*>(parallelLightInfos.data());
        auto count = static_cast<int>(parallelLightInfos.size());
        Material::SetGlobalFloatArrValue(
            "_ParallelLightInfo",
            updateBuffer,
            count * (sizeof(ParallelLightInfo) / sizeof(float)));
        Material::SetGlobalIntValue("_ParallelLightCount", count);

        struct alignas(8) PointLightInfo
        {
            Vec4 param0; // x: pos.x, y: pos.y, z: pos.z, w: radius
            Vec4 param1; // x: color.x, y: color.y, z: color.z, w: dummy
        };
    
        auto pointLightInfos = std::vector<PointLightInfo>();
        pointLightInfos.reserve(parallelLights.size());
        for (auto light : pointLights)
        {
            pointLightInfos.push_back({
                Vec4(light->owner->transform->GetPosition(), light->radius),
                Vec4(light->GetColor(), 0)
            });
        }
        updateBuffer = reinterpret_cast<float*>(pointLightInfos.data());
        count = static_cast<int>(pointLightInfos.size());
        Material::SetGlobalFloatArrValue(
            "_PointLightInfo",
            updateBuffer,
            count * (sizeof(PointLightInfo) / sizeof(float)));
        Material::SetGlobalIntValue("_PointLightCount", count);
    }
}
