﻿#pragma once
#include "Image.h"
#include "RenderPass.h"

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
};
