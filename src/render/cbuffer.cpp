#include "cbuffer.h"

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
        size = compiler.get_declared_struct_size(bufferType);
        binding = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
        
        auto memberCount = bufferType.member_types.size();
        for (auto i = 0u; i < memberCount; ++i)
        {
            CBufferParam param;
            param.name = compiler.get_member_name(bufferType.self, i);
            param.offset = compiler.type_struct_member_offset(bufferType, i);
            param.size = compiler.get_declared_struct_member_size(bufferType, i);
            param.blockNameId = name.Hash();

            params[param.name.Hash()] = param;
        }

        glUsage = usage;
    }

    CBuffer::CBuffer(CBufferLayout* layout)
    {
        this->layout = layout;
        INCREF(this->layout);
        
        glGenBuffers(1, &glUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, glUbo);
        glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizei>(layout->size), nullptr, layout->glUsage);
        glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);

        Utils::CheckGlError("创建CBuffer");
    }

    CBuffer::~CBuffer()
    {
        glDeleteBuffers(1, &glUbo);

        DECREF(this->layout);
    }

    bool CBuffer::TryGetParam(const size_t nameId, CBufferParam** result)
    {
        auto it = layout->params.find(nameId);
        if (it != layout->params.end())
        {
            if (result)
            {
                *result = &it->second;
            }
            return true;
        }

        return false;
    }

    void CBuffer::StartSync()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, glUbo);
    }

    void CBuffer::Sync(const SyncInfo& syncInfo)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, syncInfo.dstParam->offset, static_cast<GLsizei>(syncInfo.dstParam->size), syncInfo.srcData);
    }

    void CBuffer::EndSync()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
    }

    void CBuffer::Use()
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, layout->binding, glUbo);
    }
}
