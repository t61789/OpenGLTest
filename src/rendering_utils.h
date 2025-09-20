#pragma once
#include <optional>
#include <vector>

#include "const.h"


namespace op
{
    class RenderComp;
    class RenderContext;
    class RenderTexture;
    class Entity;
    class Mesh;
    class Material;
    class Matrix4x4;
    class Shader;

    class RenderingUtils
    {
    public:
        struct RenderParam
        {
            Mesh* mesh = nullptr;
            Material* material = nullptr;
            bool hasOddNegativeScale = false;
            std::optional<uint32_t> objectIndex = std::nullopt;
        };
        
        static void RenderScene(crvec<RenderComp*> renderComps);
        static void RenderEntity(const RenderComp* renderComp);
        static void Blit(crsp<RenderTexture> src, crsp<RenderTexture> dst, Material* material = nullptr);
        static void RenderMesh(cr<RenderParam> renderParam);
        static void BindDrawResources(cr<RenderParam> renderParam);
        
    private:
        static void CallGlDraw(const Mesh* mesh);
    };
}
