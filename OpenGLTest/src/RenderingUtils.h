﻿#pragma once
#include "Entity.h"
#include "RenderContext.h"

class RenderingUtils
{
public:
    static void RenderScene(const RenderContext& renderContext, const std::vector<Entity*>& renderObjs);
    static void RenderEntity(const RenderContext& renderContext, const Entity* entity);
    static void RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const glm::mat4& m);
    static void Blit(RenderTexture* src, RenderTexture* dst, Material* material);
};
