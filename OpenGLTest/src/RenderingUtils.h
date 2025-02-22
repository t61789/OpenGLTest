#pragma once
#include "RenderPipeline.h"

class RenderingUtils
{
public:
    static void RenderScene(const RenderContext& renderContext);
    static void RenderEntity(const RenderContext& renderContext, const Entity* entity);
    static void RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const glm::mat4& m);
};
