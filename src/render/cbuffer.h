#pragma once
#include <spirv_glsl.hpp>
#include "glad/glad.h"

#include "string_handle.h"

namespace op
{
    struct CBufferParam
    {
        StringHandle name;
        size_t sizeB;
        uint32_t offsetB;
        size_t blockNameId;
    };
    
    struct CBufferLayout
    {
        StringHandle name;
        size_t sizeB;
        uint32_t binding;
        GLint glUsage = GL_DYNAMIC_DRAW;

        std::unordered_map<size_t, CBufferParam> params;

        CBufferLayout() = default;
        CBufferLayout(const spirv_cross::CompilerGLSL& compiler, const spirv_cross::Resource& uniformBuffer, GLint usage = GL_DYNAMIC_DRAW);
    };
}
