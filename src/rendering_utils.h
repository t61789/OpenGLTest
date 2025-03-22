#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "objects/render_comp.h"

namespace op
{
    class RenderContext;
    class RenderTexture;
    class Entity;
    class Mesh;
    class Material;

    class RenderingUtils
    {
    public:
        static void RenderScene(const RenderContext& renderContext, const std::vector<RenderComp*>& renderComps);
        static void RenderEntity(const RenderContext& renderContext, const RenderComp* renderComp);
        static void RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const glm::mat4& m);
        static void Blit(RenderTexture* src, RenderTexture* dst, Material* material = nullptr);
    };
}
