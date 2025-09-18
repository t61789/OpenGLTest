#include "gl_cbuffer.h"

#include "gl_submit_buffer.h"
#include "common/elem_accessor_var.h"

namespace op
{
    GlCbuffer::GlCbuffer(crsp<CBufferLayout> layout)
    {
        m_layout = layout;
        m_buffer = msp<GlSubmitBuffer>(
            static_cast<uint32_t>(GL_UNIFORM_BUFFER),
            static_cast<uint32_t>(GL_DYNAMIC_DRAW),
            static_cast<uint32_t>(m_layout->sizeB),
            m_layout->binding);
    }

    GlCbuffer::~GlCbuffer() = default;

    void GlCbuffer::Bind()
    {
        m_buffer->Bind();
    }

    void GlCbuffer::BindBase()
    {
        m_buffer->BindBase();
    }

    void GlCbuffer::Submit()
    {
        m_buffer->Submit();
    }

    bool GlCbuffer::Set(const size_t nameId, const float* val, const size_t count)
    {
        return TrySetRaw(nameId, val, count * sizeof(float));
    }

    void GlCbuffer::Get(const size_t nameId, float* val, const size_t count)
    {
        if (!TryGetRaw(nameId, val, count * sizeof(float)))
        {
            THROW_ERROR("Failed to get cbuffer param")
        }
    }

    bool GlCbuffer::TryGet(const size_t nameId, float* val, const size_t count)
    {
        return TryGetRaw(nameId, val, count * sizeof(float));
    }

    bool GlCbuffer::TrySetRaw(const string_hash nameId, const void* data, uint32_t sizeB)
    {
        auto it = m_layout->params.find(nameId);
        if (it == m_layout->params.end())
        {
            return false;
        }

        auto& param = it->second;
        m_buffer->SetData(param.offsetB, std::min(param.sizeB, sizeB), data);

        return true;
    }

    bool GlCbuffer::TryGetRaw(const string_hash nameId, void* data, const uint32_t sizeB)
    {
        auto it = m_layout->params.find(nameId);
        if (it == m_layout->params.end())
        {
            return false;
        }
        
        auto& param = it->second;
        if (param.sizeB != sizeB)
        {
            return false;
        }

        m_buffer->GetData(param.offsetB, param.sizeB, data);

        return true;
    }
}
