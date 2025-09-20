#include "render_skybox_pass.h"

#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "image.h"
#include "material.h"

#include "mesh.h"
#include "rendering_utils.h"
#include "render_context.h"
#include "objects/camera_comp.h"
#include "objects/render_comp.h"
#include "objects/transform_comp.h"
#include "render/per_object_buffer.h"
#include "render/render_target_pool.h"
#include "render/texture_set.h"
#include "render/gl/gl_cbuffer.h"

namespace op
{
    RenderSkyboxPass::RenderSkyboxPass()
    {
        auto desc = ImageDescriptor::GetDefault();
        desc.needFlipVertical = false;
        
        m_skyboxCubeTexture = Image::LoadCubeFromFile("built_in/texture/skybox", "png", desc);
        GetGlobalTextureSet()->SetTexture(SKYBOX_TEX, m_skyboxCubeTexture);
        m_sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
        m_skyboxMat = Material::LoadFromFile("materials/skybox_mat.json");
        
        m_objectIndex = GetGR()->GetPerObjectBuffer()->Register();
    }

    RenderSkyboxPass::~RenderSkyboxPass()
    {
        GetGR()->GetPerObjectBuffer()->UnRegister(m_objectIndex);
    }

    void RenderSkyboxPass::Execute()
    {
        ZoneScoped;
        
        auto camera = GetRC()->camera;
        if(camera == nullptr)
        {
            return;
        }

        auto usingGBufferRenderTarget = GetRC()->UsingGBufferRenderTarget();

        PerObjectBuffer::Elem submitBuffer;
        submitBuffer.localToWorld = Matrix4x4::TRS(camera->GetOwner()->transform->GetWorldPosition(), Quaternion::Identity(), Vec3::One());
        submitBuffer.worldToLocal = submitBuffer.localToWorld.Inverse();
        GetGR()->GetPerObjectBuffer()->SubmitData(m_objectIndex, submitBuffer);

        RenderingUtils::RenderMesh({
            m_sphereMesh.get(),
            m_skyboxMat.get(),
            false,
            m_objectIndex
        });
    }
}
