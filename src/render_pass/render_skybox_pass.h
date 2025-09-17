#pragma once
#include "const.h"
#include "i_render_pass.h"

namespace op
{
    class Material;
    class Mesh;
    class Image;

    class RenderSkyboxPass final : public IRenderPass
    {
    public:
        RenderSkyboxPass();
        ~RenderSkyboxPass() override;
        RenderSkyboxPass(const RenderSkyboxPass& other) = delete;
        RenderSkyboxPass(RenderSkyboxPass&& other) noexcept = delete;
        RenderSkyboxPass& operator=(const RenderSkyboxPass& other) = delete;
        RenderSkyboxPass& operator=(RenderSkyboxPass&& other) noexcept = delete;

        std::string GetName() override { return "Render Skybox Pass";}
        void Execute() override;

    private:
        sp<Image> m_skyboxCubeTexture = nullptr;
        sp<Mesh> m_sphereMesh = nullptr;
        sp<Material> m_skyboxMat = nullptr;
        uint32_t m_objectIndex = ~0u;
    };
}
