#pragma once
#include <cstdint>
#include <memory>

#include "i_gl_resource.h"

namespace op
{
    class GlBuffer : public IGlResource, public std::enable_shared_from_this<GlBuffer>
    {
    public:
        explicit GlBuffer(uint32_t type, uint32_t slot = ~0u);
        ~GlBuffer();

        uint32_t GetId() const { return m_id;}
        uint32_t GetType() const { return m_type;}
        uint32_t GetSize() const { return m_sizeB;}

        void Bind();
        void BindBase();
        void Delete();
        void SetData(uint32_t usage, uint32_t sizeB, const void* data);
        void SetSubData(uint32_t offsetB, uint32_t sizeB, const void* data);
        void* MapBuffer(uint32_t access);
        void UnMapBuffer();

    private:
        uint32_t m_id = 0;
        uint32_t m_type = 0;
        uint32_t m_slot = ~0u;
        uint32_t m_sizeB = 0;
        uint32_t m_usage = 0;
        bool m_mapping = false;
    };
}
