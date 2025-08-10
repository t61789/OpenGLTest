#pragma once
#include <spirv_glsl.hpp>
#include "glad/glad.h"

#include "shared_object.h"
#include "string_handle.h"

namespace op
{
    struct CBufferParam
    {
        StringHandle name;
        size_t size;
        uint32_t offset;
        size_t blockNameId;
    };
    
    struct CBufferLayout : SharedObject
    {
        StringHandle name;
        size_t size;
        uint32_t binding;
        GLint glUsage = GL_DYNAMIC_DRAW;

        std::unordered_map<size_t, CBufferParam> params;
        
        CBufferLayout(const spirv_cross::CompilerGLSL& compiler, const spirv_cross::Resource& uniformBuffer, GLint usage = GL_DYNAMIC_DRAW);
    };

    class CBuffer : public SharedObject
    {
    public:
        struct SyncInfo
        {
            void* srcData;
            CBufferParam* dstParam;
        };
        
        CBufferLayout* layout = nullptr;
        GLuint glUbo = GL_NONE;

        explicit CBuffer(CBufferLayout* layout);
        ~CBuffer() override;

        bool TryGetParam(size_t nameId, CBufferParam** result);
        void StartSync();
        void Sync(const SyncInfo& syncInfo);
        void EndSync();
        void Use();
    };
}
