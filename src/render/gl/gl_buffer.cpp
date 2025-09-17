#include "gl_buffer.h"

#include <cassert>
#include <glad/glad.h>

#include "gl_state.h"

namespace op
{
    GlBuffer::GlBuffer(const uint32_t type, const uint32_t slot)
    {
        m_type = type;
        m_slot = slot;
        m_id = GlState::GlGenBuffer();
    }

    GlBuffer::~GlBuffer()
    {
        Delete();
    }

    void GlBuffer::Delete()
    {
        assert(!m_mapping);

        GlState::GlDeleteBuffer(m_id);
    }

    void GlBuffer::SetData(const uint32_t usage, const uint32_t sizeB, const void* data)
    {
        assert(!m_mapping);

        GlState::Ins()->BindBuffer(shared_from_this());
        GlState::GlBufferData(m_type, sizeB, data, usage);

        m_sizeB = sizeB;
        m_usage = usage;
    }
    
    void GlBuffer::SetSubData(const uint32_t offsetB, const uint32_t sizeB, const void* data)
    {
        assert(!m_mapping && offsetB + sizeB <= m_sizeB);

        GlState::Ins()->BindBuffer(shared_from_this());
        GlState::GlBufferSubData(m_type, offsetB, sizeB, data);
    }

    void* GlBuffer::MapBuffer(const uint32_t access)
    {
        assert(!m_mapping && m_sizeB > 0);

        GlState::Ins()->UseGlResource(shared_from_this());
        GlState::Ins()->BindBuffer(shared_from_this());
        m_mapping = true;
        return GlState::GlMapBuffer(m_type, access);
    }

    void GlBuffer::UnMapBuffer()
    {
        assert(!m_mapping);

        GlState::Ins()->EndUseGlResource(shared_from_this());
        m_mapping = false;
        GlState::GlUnmapBuffer(m_type);
    }

    void GlBuffer::Bind()
    {
        assert(!m_mapping);

        GlState::Ins()->BindBuffer(shared_from_this());
    }

    void GlBuffer::BindBase()
    {
        assert(!m_mapping);
        assert(m_slot != ~0u);

        GlState::Ins()->BindBufferBase(shared_from_this(), m_slot);
    }
}
