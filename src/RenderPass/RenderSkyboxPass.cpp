#include "RenderSkyboxPass.h"

#include "glm/ext/matrix_transform.hpp"

#include "RenderingUtils.h"

RenderSkyboxPass::RenderSkyboxPass(RenderContext* renderContext) : RenderPass(renderContext)
{
    auto desc = ImageDescriptor::GetDefault();
    desc.needFlipVertical = false;
    m_skyboxCubeTexture = Image::LoadCubeFromFile("textures/skybox", "jpg", desc);
    INCREF(m_skyboxCubeTexture);
    Material::SetGlobalTextureValue("_SkyboxTex", m_skyboxCubeTexture);
    m_sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
    INCREF(m_sphereMesh);
    m_skyboxMat = Material::LoadFromFile("materials/skybox_mat.json");
    INCREF(m_skyboxMat);
}

RenderSkyboxPass::~RenderSkyboxPass()
{
    DECREF(m_skyboxCubeTexture);
    DECREF(m_sphereMesh);
    DECREF(m_skyboxMat);
}

std::string RenderSkyboxPass::GetName()
{
    return "Render Skybox Pass";
}

void RenderSkyboxPass::Execute()
{
    auto camera = m_renderContext->camera;
    if(camera == nullptr || m_sphereMesh == nullptr || m_skyboxMat == nullptr)
    {
        return;
    }

    RenderTarget::Get(*m_renderContext->gBufferDesc)->Use();
    
    auto m = glm::mat4(1);
    m = translate(m, camera->position);
    m = scale(m, glm::vec3(1));

    RenderingUtils::RenderMesh(*m_renderContext, m_sphereMesh, m_skyboxMat, m);
}
