#pragma once
#include <set>
#include <glad/glad.h>
#include "shared_object.h"

namespace op
{
    class StructuredBuffer : public SharedObject
    {
    public:
        StructuredBuffer(uint32_t stride, uint32_t count, uint32_t binding);
        ~StructuredBuffer() override;
        void Resize(uint32_t count);
        void Use();
        
        uint32_t GetStride() const { return m_stride; }
        uint32_t GetCount() const { return m_count; }
        
        void SetData(uint32_t index, const void* data);
        void SyncData();
        void SyncDataForce();

    private:
        GLuint m_glBuffer = GL_NONE;
        uint32_t m_binding;
        uint32_t m_stride;
        uint32_t m_count;
        uint8_t* m_data;
        std::set<uint32_t> m_dirty;

        void CreateBuffer(uint32_t size);
    };
}
