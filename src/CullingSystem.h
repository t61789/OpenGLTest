﻿#pragma once
#include <vector>

#include "Gui.h"

class RenderContext;
class Bounds;
class RenderComp;
class Object;

class CullingSystem : public IGui
{
public:
    explicit CullingSystem(RenderContext* renderContext);
    void Cull();

    void OnDrawConsoleGui() override;

private:
    RenderContext* m_renderContext = nullptr;
    std::vector<Bounds> m_bounds;

    bool CullOnce(const Bounds& bounds, const std::array<glm::vec4, 6>& planes);
    
    static std::vector<Bounds> GetWorldSpaceAABB(const std::vector<RenderComp*>& renderComps);
    static std::array<glm::vec4, 6> GetFrustumPlanes(const glm::mat4& vp);
    static bool FrustumCull(const Bounds& bounds, const std::array<glm::vec4, 6>& planes);
};
