#include "RenderContext.h"

#include "glm/ext/matrix_clip_space.hpp"

#include "Material.h"
#include "RenderTexture.h"
#include "SharedObject.h"
#include "Objects/CameraComp.h"

RenderContext::~RenderContext()
{
    for (auto it : m_rts)
    {
        DECREF(it.second);
    }

    delete allSceneObjs;
    delete allRenderObjs;
    delete visibleRenderObjs;
}

void RenderContext::SetViewProjMatrix(const CameraComp* cam)
{
    auto cameraLocalToWorld = cam->owner->GetLocalToWorld();
    auto viewMatrix = glm::inverse(cameraLocalToWorld);
    auto projectionMatrix = glm::perspective(
        glm::radians(cam->fov * 0.5f),
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight),
        cam->nearClip,
        cam->farClip);
    SetViewProjMatrix(viewMatrix, projectionMatrix);
}

void RenderContext::SetViewProjMatrix(const glm::mat4& view, const glm::mat4& proj)
{
    vMatrix = view;
    pMatrix = proj;
    vpMatrix = proj * view;
    Material::SetGlobalMat4Value("_VP", vpMatrix);
    Material::SetGlobalMat4Value("_IVP", inverse(vpMatrix));
}

void RenderContext::RegisterRt(RenderTexture* rt)
{
    auto it = m_rts.find(rt->desc.name);
    if (it != m_rts.end())
    {
        DECREF(it->second);
    }
    m_rts[rt->desc.name] = rt;
    INCREF(rt);
}

void RenderContext::UnRegisterRt(const RenderTexture* rt)
{
    auto it = m_rts.find(rt->desc.name);
    if (it != m_rts.end())
    {
        DECREF(it->second);
        m_rts.erase(it);
    }
}

RenderTexture* RenderContext::GetRt(const std::string& name)
{
    auto it = m_rts.find(name);
    if (it != m_rts.end())
    {
        return it->second;
    }
    return nullptr;
}
