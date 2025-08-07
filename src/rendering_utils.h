#pragma once

#include <vector>

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
        static void RenderScene(const RenderContext& renderContext, const std::vector<RenderComp*>& renderComps, unsigned int buffer);
        static void RenderEntity(const RenderContext& renderContext, const RenderComp* renderComp, unsigned int buffer);
        static void RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const Matrix4x4& m, const Matrix4x4& im = Matrix4x4::Identity());
        static void Blit(RenderTexture* src, RenderTexture* dst, Material* material = nullptr);
        
    private:
        static void CallGlDraw(const Mesh* mesh);
    };
}
