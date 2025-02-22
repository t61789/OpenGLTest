#pragma once
#include "Image.h"
#include "RenderPass.h"

class RenderSkyboxPass : public RenderPass
{
public:
    RenderSkyboxPass();
    ~RenderSkyboxPass() override;
    std::string GetName() override;
    void Execute(RenderContext& renderContext) override;

private:
    Image* m_skyboxCubeTexture = nullptr;
    Mesh* m_sphereMesh = nullptr;
    Material* m_skyboxMat = nullptr;
};
