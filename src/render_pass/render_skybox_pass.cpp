#include "render_skybox_pass.h"

#include <tracy/Tracy.hpp>

#include "shared_object.h"
#include "image.h"

#include "mesh.h"
#include "render_target.h"
#include "rendering_utils.h"
#include "objects/camera_comp.h"
#include "objects/render_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    RenderSkyboxPass::RenderSkyboxPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        auto desc = ImageDescriptor::GetDefault();
        desc.needFlipVertical = false;
        m_skyboxCubeTexture = Image::LoadCubeFromFile("built_in/texture/skybox", "png", desc);
        INCREF(m_skyboxCubeTexture);
        m_sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
        INCREF(m_sphereMesh);
        GET_GLOBAL_CBUFFER->Set(SKYBOX_TEX, m_skyboxCubeTexture);
        m_skyboxMat = Material::LoadFromFile("materials/skybox_mat.json");
        INCREF(m_skyboxMat);
        m_objectIndex = GetGR()->perObjectBuffer->BindObject();
    }

    RenderSkyboxPass::~RenderSkyboxPass()
    {
        GetGR()->perObjectBuffer->UnbindObject(m_objectIndex);
        
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
        ZoneScoped;
        
        auto camera = m_renderContext->camera;
        if(camera == nullptr || m_sphereMesh == nullptr || m_skyboxMat == nullptr)
        {
            return;
        }

        RenderTarget::Get(*m_renderContext->gBufferDesc)->Use();
    
        m_submitBuffer.localToWorld = Matrix4x4::TRS(camera->GetOwner()->transform->GetWorldPosition(), Quaternion::Identity(), Vec3::One());
        m_submitBuffer.worldToLocal = m_submitBuffer.localToWorld.Inverse();
        GetGR()->perObjectBuffer->SubmitData(m_objectIndex, &m_submitBuffer);

        RenderingUtils::RenderMesh({
            m_sphereMesh,
            m_skyboxMat,
            &Matrix4x4::Identity(),
            &Matrix4x4::Identity(),
            m_objectIndex
        });
    }
}