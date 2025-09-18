#pragma once
#include <spirv_glsl.hpp>

#include "string_handle.h"

namespace op
{
    class GlShader;

    struct CBufferParam
    {
        StringHandle name;
        uint32_t sizeB;
        uint32_t offsetB;
        size_t blockNameId;
    };
    
    struct CBufferLayout
    {
        StringHandle name;
        uint32_t sizeB;
        uint32_t binding;

        std::unordered_map<size_t, CBufferParam> params;

        CBufferLayout() = default;
        CBufferLayout(const GlShader* shader, const char* uniformBufferName);
    };
}
