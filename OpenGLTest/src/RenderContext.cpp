#include "RenderContext.h"

#include <ext/matrix_clip_space.hpp>

void RenderContext::SetViewProjMatrix(const Camera* cam)
{
    auto cameraLocalToWorld = cam->GetLocalToWorld();
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

