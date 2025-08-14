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
        static void RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const Matrix4x4& m, const Matrix4x4& im = Matrix4x4::Identity(), MaterialNew* matNew = nullptr);
        static void Blit(RenderTexture* src, RenderTexture* dst, Material* material = nullptr);
        static void Blit0(RenderTexture* src, RenderTexture* dst, const RenderContext& rc, MaterialNew* material = nullptr); // TODO new material
        
    private:
        static void CallGlDraw(const Mesh* mesh);
    };
}
