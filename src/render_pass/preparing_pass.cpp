#include "preparing_pass.h"

#include "gui.h"
#include "indirect_lighting.h"

#include "scene.h"
#include "objects/camera_comp.h"
#include "objects/light_comp.h"
#include "objects/transform_comp.h"
#include "const.h"
#include "material.h"
#include "objects/render_comp.h"
#include "render/render_target.h"
#include "render/gl/gl_cbuffer.h"

namespace op
{
    void PreparingPass::Execute()
    {
        ZoneScoped;
        
        if(GetRC()->camera == nullptr)
        {
            return;
        }

        PrepareMatrices();

        PrepareViewport();

        UpdateTransforms();

        ClearGBuffers();

        PrepareLightInfos();

        SetAmbientColors();

        SetFogParams();
    }

    void PreparingPass::DrawConsoleUi()
    {
        auto scene = GetRC()->scene;
        if (!scene)
        {
            return;
        }

        if (ImGui::CollapsingHeader("Ambient Lights"))
        {
            scene->ambientLightColorSky = Gui::SliderFloat3("GradientSky", scene->ambientLightColorSky, 0.0f, 10.0f, "%.2f");
            scene->ambientLightColorEquator = Gui::SliderFloat3("GradientEquator", scene->ambientLightColorEquator, 0.0f, 10.0f, "%.2f");
            scene->ambientLightColorGround = Gui::SliderFloat3("GradientGround", scene->ambientLightColorGround, 0.0f, 10.0f, "%.2f");
            ImGui::SliderFloat("Fog Intensity", &scene->fogIntensity, 0.0f, 0.005f, "%.5f");
            scene->fogColor = Gui::SliderFloat3("Fog Color", scene->fogColor, 0.0f, 1.0f, "%.2f");
        }
        
        if (ImGui::CollapsingHeader("Main Light Shadow"))
        {
            ImGui::SliderFloat("Range", &GetRC()->mainLightShadowRange, 0.1f, 200.0f, "%.2f");
        }
    }

    void PreparingPass::PrepareMatrices()
    {
        ZoneScoped;
        
        // Common camera
        GetRC()->PopViewProjMatrix();
        GetRC()->mainVPInfo = GetRC()->camera->CreateVPMatrix();
        GetRC()->mainVPInfo->UpdateFrustumPlanes();
        GetRC()->PushViewProjMatrix(GetRC()->mainVPInfo);
        auto commonCullJob = GetGR()->GetCullingBuffer()->CreateCullJob(GetRC()->mainVPInfo->frustumPlanes.value(), ViewGroup::COMMON);
        auto commonEncodingJob = GetGR()->GetBatchRenderUnit()->CreateEncodingJob(ViewGroup::COMMON);
        commonCullJob->AppendNext(commonEncodingJob);
        
        // Shadow camera
        auto lightDirection = GetRC()->mainLight ?
            -GetRC()->mainLight->GetOwner()->transform->GetLocalToWorld().Forward() :
            Vec3::One().Normalize();
        GetRC()->shadowVPInfo = GetRC()->camera->CreateShadowVPMatrix(lightDirection);
        GetRC()->shadowVPInfo->UpdateFrustumPlanes();
        auto shadowCullJob = GetGR()->GetCullingBuffer()->CreateCullJob(GetRC()->shadowVPInfo->frustumPlanes.value(), ViewGroup::SHADOW);
        auto shadowEncodingJob = GetGR()->GetBatchRenderUnit()->CreateEncodingJob(ViewGroup::SHADOW);
        shadowCullJob->AppendNext(shadowEncodingJob);

        commonCullJob->AppendNext(shadowCullJob);

        GetGR()->GetJobScheduler()->Schedule(commonCullJob);

        GetGR()->GetCullingSystem()->Cull();
    }

    void PreparingPass::PrepareViewport()
    {
        ZoneScoped;
        
        auto viewportSize = Vec4(
            static_cast<float>(GetRC()->screenWidth),
            static_cast<float>(GetRC()->screenHeight),
            0, 0);
        
        auto perViewCBuffer = GetGR()->GetPredefinedCbuffer(PER_VIEW_CBUFFER);
        perViewCBuffer->Set(VIEWPORT_SIZE, viewportSize);
    }

    void PreparingPass::UpdateTransforms()
    {
        ZoneScoped;
        
        // 把需要渲染的objs的矩阵都上传一下
        for (auto renderComp : GetRC()->visibleRenderObjs)
        {
            renderComp->UpdateTransform();
        }
    }

    void PreparingPass::ClearGBuffers()
    {
        ZoneScoped;
        
        auto usingGBufferRenderTarget = GetRC()->UsingGBufferRenderTarget();
        vec<Vec4> clearColors = {
            Vec4(0.5f),
            Vec4(0.0f),
            Vec4(1.0f),
        };
        usingGBufferRenderTarget.Get()->Clear(clearColors, 1.0f);
    }

    void PreparingPass::PrepareLightInfos()
    {
        ZoneScoped;
        
        GetRC()->mainLight = nullptr;

        vec<LightComp*> parallelLights;
        vec<LightComp*> pointLights;

        auto globalCBuffer = GetGR()->GetPredefinedCbuffer(GLOBAL_CBUFFER);

        for (const auto& lightPtr : *GetRC()->lights)
        {
            auto light = lightPtr.lock();
            constexpr int maxPointLights = 16;
            constexpr int maxParallelLights = 4;
            if (light->lightType == 0 && parallelLights.size() < maxParallelLights)
            {
                if (!GetRC()->mainLight)
                {
                    GetRC()->mainLight = light.get();
                    globalCBuffer->Set(MAIN_LIGHT_DIRECTION, Vec4(-light->GetOwner()->transform->GetLocalToWorld().Forward(), 0));
                }

                parallelLights.push_back(light.get());
            }
            else if (light->lightType == 1 && pointLights.size() < maxPointLights)
            {
                pointLights.push_back(light.get());
            }
        }

        struct alignas(16) ParallelLightInfo
        {
            Vec4 param0; // x: dir.x, y: dir.y, z: dir.z, w: dummy
            Vec4 param1; // x: color.x, y: color.y, z: color.z, w: dummy
        };

        auto parallelLightInfos = std::vector<ParallelLightInfo>();
        parallelLightInfos.reserve(parallelLights.size());
        for (auto light : parallelLights)
        {
            parallelLightInfos.push_back({
                Vec4(-light->GetOwner()->transform->GetLocalToWorld().Forward(), 0),
                Vec4(light->GetColor(), 0)
            });
        }
        auto updateBuffer = reinterpret_cast<float*>(parallelLightInfos.data());
        auto count = static_cast<int>(parallelLightInfos.size());
        globalCBuffer->Set(
            PARALLEL_LIGHT_INFO,
            updateBuffer,
            count * (sizeof(ParallelLightInfo) / sizeof(float)));
        globalCBuffer->Set(PARALLEL_LIGHT_COUNT, count);

        struct alignas(16) PointLightInfo
        {
            Vec4 param0; // x: pos.x, y: pos.y, z: pos.z, w: radius
            Vec4 param1; // x: color.x, y: color.y, z: color.z, w: dummy
        };
    
        auto pointLightInfos = std::vector<PointLightInfo>();
        pointLightInfos.reserve(parallelLights.size());
        for (auto light : pointLights)
        {
            pointLightInfos.push_back({
                Vec4(light->GetOwner()->transform->GetPosition(), light->radius),
                Vec4(light->GetColor(), 0)
            });
        }
        updateBuffer = reinterpret_cast<float*>(pointLightInfos.data());
        count = static_cast<int>(pointLightInfos.size());
        globalCBuffer->Set(
            POINT_LIGHT_INFO,
            updateBuffer,
            count * (sizeof(PointLightInfo) / sizeof(float)));
        globalCBuffer->Set(POINT_LIGHT_COUNT, count);
    }

    void PreparingPass::SetAmbientColors()
    {
        ZoneScoped;
        
        auto scene = GetRC()->scene;
        IndirectLighting::SetGradientAmbientColor(
            scene->ambientLightColorSky,
            scene->ambientLightColorEquator,
            scene->ambientLightColorGround);
    }

    void PreparingPass::SetFogParams()
    {
        ZoneScoped;
        
        auto scene = GetRC()->scene;
        auto globalCBuffer = GetGR()->GetPredefinedCbuffer(GLOBAL_CBUFFER);
        globalCBuffer->Set(FOG_INTENSITY, scene->fogIntensity);
        globalCBuffer->Set(FOG_COLOR, Vec4(scene->fogColor, 0));
    }
}
