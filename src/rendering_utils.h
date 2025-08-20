#pragma once

#include <vector>

#include "objects/render_comp.h"

namespace op
{
    class RenderContext;
    class RenderTexture;
    class Entity;
    class Mesh;

    class RenderingUtils
    {
    public:
        static void RenderScene(const std::vector<RenderComp*>& renderComps);
        static void RenderEntity(const RenderComp* renderComp);
        static void RenderMesh(Mesh* mesh, Material* mat, const Matrix4x4& m, const Matrix4x4& im = Matrix4x4::Identity());
        static void Blit(RenderTexture* src, RenderTexture* dst, Material* material = nullptr);
        
    private:
        static void ApplyTextures(Material* material, Shader* shader);
        static void CallGlDraw(const Mesh* mesh);
        static void BindDrawResources(Shader* shader, Mesh* mesh, Material* material);
    };
}
