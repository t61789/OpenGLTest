#include "per_object_buffer.h"

#include "utils.h"
#include "gl/gl_submit_buffer.h"

namespace op
{
    PerObjectBuffer::PerObjectBuffer(const uint32_t capacity, const uint32_t slot)
    {
        m_submitBuffer = msp<GlSubmitBuffer>(
            static_cast<uint32_t>(GL_SHADER_STORAGE_BUFFER),
            static_cast<uint32_t>(GL_DYNAMIC_DRAW),
            static_cast<uint32_t>(capacity * sizeof(Elem)),
            slot);
        m_bufferAccessor = mup<ElemAccessorFixed<Elem>>(m_submitBuffer);
        m_bufferAccessor->Resize(capacity);
        m_objectInfos.resize(capacity);
    }

    uint32_t PerObjectBuffer::Register()
    {
        while (true)
        {
            assert(m_nextEmpty <= m_bufferAccessor->Size());
            
            if (m_nextEmpty == m_bufferAccessor->Size())
            {
                Reserve(m_bufferAccessor->Size() * 2);
            }
            
            auto& info = m_objectInfos[m_nextEmpty++];
            if (!info.enable)
            {
                info.enable = true;
                return m_nextEmpty - 1;
            }
        }
    }

    void PerObjectBuffer::UnRegister(const uint32_t index)
    {
        auto& info = m_objectInfos[index];
        info.enable = false;
        m_nextEmpty = std::min(m_nextEmpty, index);
    }

    void PerObjectBuffer::Reserve(const uint32_t count)
    {
        if (count <= m_bufferAccessor->Size())
        {
            return;
        }
        m_bufferAccessor->Reserve(count);
        m_bufferAccessor->Resize(m_bufferAccessor->Capacity());
        m_objectInfos.resize(m_bufferAccessor->Capacity());
    }

    void PerObjectBuffer::SubmitData(const uint32_t index, cr<Elem> data)
    {
        m_bufferAccessor->Set(index, data);
    }

    void PerObjectBuffer::Use()
    {
        m_submitBuffer->BindBase();
    }
}
