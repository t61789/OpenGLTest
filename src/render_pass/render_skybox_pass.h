#pragma once
#include "material.h"
#include "render_pass.h"

namespace op
{
    class Mesh;
    class Image;

    class RenderSkyboxPass : public RenderPass
    {
    public:
        RenderSkyboxPass(RenderContext* renderContext);
        ~RenderSkyboxPass() override;
        std::string GetName() override;
        void Execute() override;

    private:
        Image* m_skyboxCubeTexture = nullptr;
        Mesh* m_sphereMesh = nullptr;
        Material* m_skyboxMat = nullptr;
        uint32_t m_objectIndex = ~0u;
    };
}
