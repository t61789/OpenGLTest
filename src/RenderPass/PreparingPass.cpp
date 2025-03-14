#include "PreparingPass.h"

#include "RenderTarget.h"
#include "Material.h"
#include "Objects/CameraComp.h"
#include "Objects/LightComp.h"
#include "Objects/TransformComp.h"

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

    Material::SetGlobalVector4Value("_CameraPositionWS", glm::vec4(m_renderContext->camera->owner->transform->GetPosition(), 0));

    std::vector<glm::vec4> clearColors = {
        glm::vec4(0.5f),
        glm::vec4(0.0f),
        glm::vec4(1.0f),
    };
    RenderTarget::Get(*m_renderContext->gBufferDesc)->Clear(clearColors, 1.0f);

    auto viewportSize = glm::vec4(m_renderContext->screenWidth, m_renderContext->screenHeight, 0, 0);
    Material::SetGlobalVector4Value("_ViewportSize", viewportSize);

    PrepareLightInfos();
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
                Material::SetGlobalVector4Value("_MainLightDirection", glm::vec4(-light->owner->transform->Forward(), 0));
            }

            parallelLights.push_back(light);
        }
        else if (light->lightType == 1 && pointLights.size() < maxPointLights)
        {
            pointLights.push_back(light);
        }
    }

    struct alignas(4) ParallelLightInfo
    {
        glm::vec3 direction;
        glm::vec3 color;
    };

    auto parallelLightInfos = std::vector<ParallelLightInfo>();
    parallelLightInfos.reserve(parallelLights.size());
    for (auto light : parallelLights)
    {
        parallelLightInfos.push_back({ -light->owner->transform->Forward(), light->color });
    }
    auto updateBuffer = reinterpret_cast<float*>(parallelLightInfos.data());
    auto count = static_cast<int>(parallelLightInfos.size());
    Material::SetGlobalFloatArrValue(
        "_ParallelLightInfo",
        updateBuffer,
        count * (sizeof(ParallelLightInfo) / sizeof(float)));
    Material::SetGlobalIntValue("_ParallelLightCount", count);

    struct alignas(4) PointLightInfo
    {
        glm::vec3 position;
        float radius;
        glm::vec3 color;
    };
    
    auto pointLightInfos = std::vector<PointLightInfo>();
    pointLightInfos.reserve(parallelLights.size());
    for (auto light : pointLights)
    {
        pointLightInfos.push_back({ light->owner->transform->GetPosition(), light->radius, light->color });
    }
    updateBuffer = reinterpret_cast<float*>(pointLightInfos.data());
    count = static_cast<int>(pointLightInfos.size());
    Material::SetGlobalFloatArrValue(
        "_PointLightInfo",
        updateBuffer,
        count * (sizeof(PointLightInfo) / sizeof(float)));
}
