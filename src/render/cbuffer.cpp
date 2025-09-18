#include "cbuffer.h"

#include "render_context.h"
#include "utils.h"
#include "gl/gl_shader.h"
#include "gl/gl_state.h"

namespace op
{
    CBufferLayout::CBufferLayout(
        const GlShader* shader,
        const char* uniformBufferName)
    {
        name = uniformBufferName;
        
        auto glShaderId = shader->GetId();

        // Get Cbuffer index
        auto blockIndex = GlState::GlGetUniformBlockIndex(glShaderId, name.CStr());
        assert(blockIndex != GL_INVALID_INDEX);

        // Get Cbuffer binding
        int32_t bindingTemp;
        GlState::GlGetActiveUniformBlockiv(glShaderId, blockIndex, GL_UNIFORM_BLOCK_BINDING, &bindingTemp);
        binding = bindingTemp;

        // Get Cbuffer size
        int32_t sizeBTemp;
        GlState::GlGetActiveUniformBlockiv(glShaderId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &sizeBTemp);
        sizeB = sizeBTemp;

        // Get uniform count
        int32_t uniformCount;
        GlState::GlGetActiveUniformBlockiv(glShaderId, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniformCount);

        // Get uniform indices in specified block
        vec<int32_t> uniformIndices(uniformCount);
        GlState::GlGetActiveUniformBlockiv(glShaderId, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndices.data());

        // Get uniform offsets
        vec<int32_t> uniformOffsets(uniformCount);
        GlState::GlGetActiveUniformsiv(glShaderId, uniformCount, reinterpret_cast<GLuint*>(uniformIndices.data()), GL_UNIFORM_OFFSET, uniformOffsets.data());

        // Get uniform types
        vec<int32_t> uniformTypes(uniformCount);
        GlState::GlGetActiveUniformsiv(glShaderId, uniformCount, reinterpret_cast<GLuint*>(uniformIndices.data()), GL_UNIFORM_TYPE, uniformTypes.data());
        
        // Get uniform size. Got 1 for non-array uniforms and element count for array uniforms, not the byte size
        vec<int32_t> uniformSizeB(uniformCount);
        GlState::GlGetActiveUniformsiv(glShaderId, uniformCount, reinterpret_cast<GLuint*>(uniformIndices.data()), GL_UNIFORM_SIZE, uniformSizeB.data());

        for (auto i = 0; i < uniformCount; ++i)
        {
            GLchar uniformName[256];
            GLsizei length;
            GlState::GlGetActiveUniformName(glShaderId, uniformIndices[i], sizeof(name), &length, uniformName);

            // The value of the uniformName is similar to PerViewCBuffer._VP, so we need to remove the prefix
            auto realUniformName = replace(uniformName, name.Str() + std::string("."), "");
            if (ends_with(realUniformName, "[0]"))
            {
                realUniformName = replace(realUniformName, "[0]", "");
            }
            
            CBufferParam param;
            param.name = realUniformName;
            param.offsetB = uniformOffsets[i];
            param.sizeB = uniformSizeB[i] * get_type_size(uniformTypes[i]);
            param.blockNameId = name.Hash();

            params[param.name.Hash()] = param;
        }
    }
}
