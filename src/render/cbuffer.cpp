#include "cbuffer.h"

#include "render_context.h"
#include "utils.h"

namespace op
{
    CBufferLayout::CBufferLayout(
        const spirv_cross::CompilerGLSL& compiler,
        const spirv_cross::Resource& uniformBuffer,
        const GLint usage)
    {
        const auto& bufferType = compiler.get_type(uniformBuffer.type_id);
        
        name = uniformBuffer.name;
        sizeB = compiler.get_declared_struct_size(bufferType);
        binding = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
        
        auto memberCount = bufferType.member_types.size();
        for (auto i = 0u; i < memberCount; ++i)
        {
            CBufferParam param;
            param.name = compiler.get_member_name(bufferType.self, i);
            param.offsetB = compiler.type_struct_member_offset(bufferType, i);
            param.sizeB = compiler.get_declared_struct_member_size(bufferType, i);
            param.blockNameId = name.Hash();

            params[param.name.Hash()] = param;
        }

        glUsage = usage;
    }
}
