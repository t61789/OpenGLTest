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
        struct RenderParam
        {
            Mesh* mesh = nullptr;
            Material* material = nullptr;
            const Matrix4x4* localToWorld = nullptr;
            const Matrix4x4* worldToLocal = nullptr;
            std::optional<uint32_t> objectIndex = std::nullopt;
        };
        
        static void RenderScene(const std::vector<RenderComp*>& renderComps);
        static void RenderEntity(const RenderComp* renderComp);
        static void RenderMesh(const RenderParam& renderParam);
        static void Blit(RenderTexture* src, RenderTexture* dst, Material* material = nullptr);
        
    private:
        static void ApplyTextures(Material* material, Shader* shader);
        static void CallGlDraw(const Mesh* mesh);
        static void BindDrawResources(const RenderParam& renderParam);
    };
}
