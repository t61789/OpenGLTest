#include "gl_submit_buffer.h"

#include "gl_buffer.h"
#include "gl_state.h"
#include "gl_vertex_array.h"
#include "utils.h"

namespace op
{
    GlSubmitBuffer::GlSubmitBuffer(const uint32_t type, const uint32_t usage, const uint32_t sizeB, const uint32_t slot)
    {
        m_sizeB = sizeB;
        m_usage = usage;
        m_slot = slot;
        m_data = new uint8_t[sizeB];
        
        m_glBuffer = msp<GlBuffer>(type, slot);
        m_glBuffer->SetData(usage, sizeB, nullptr);
    }

    GlSubmitBuffer::~GlSubmitBuffer()
    {
        m_glBuffer.reset();
        delete[] m_data;
    }

    void GlSubmitBuffer::Bind()
    {
        Submit();

        m_glBuffer->Bind();
    }

    void GlSubmitBuffer::BindBase()
    {
        Submit();

        m_glBuffer->BindBase();
    }

    void GlSubmitBuffer::Resize(const uint32_t sizeB)
    {
        assert(sizeB > 0);

        auto newData = new uint8_t[sizeB];
        memcpy(newData, m_data, std::min(m_sizeB, sizeB));
        delete[] m_data;
        
        m_data = newData;
        m_sizeB = sizeB;
    }

    bool GlSubmitBuffer::SetData(const uint32_t offsetB, const uint32_t sizeB, const void* data)
    {
        assert(offsetB + sizeB <= m_sizeB);
        
        m_dirtyBlocks.push_back({ offsetB, sizeB });
        memcpy(m_data + offsetB, data, sizeB);

        return true;
    }

    bool GlSubmitBuffer::GetData(const uint32_t offsetB, const uint32_t sizeB, void* data)
    {
        if (offsetB + sizeB > m_sizeB)
        {
            THROW_ERRORF("Block out of range: offset %u, size %u", offsetB, sizeB)
        }

        memcpy(data, m_data + offsetB, sizeB);

        return true;
    }

    void GlSubmitBuffer::Submit()
    {
        if (m_glBuffer->GetSize() != m_sizeB)
        {
            m_glBuffer->SetData(m_usage, m_sizeB, m_data);
            m_dirtyBlocks.clear();
            return;
        }
        
        if (m_dirtyBlocks.empty())
        {
            return;
        }
        
        std::sort(m_dirtyBlocks.begin(), m_dirtyBlocks.end(), [](cr<BlockInfo> a, cr<BlockInfo> b)
        {
            if (a.offsetB == b.offsetB)
            {
                return a.sizeB < b.sizeB;
            }

            return a.offsetB < b.offsetB;
        });

        uint32_t curOffsetB = 0, curSizeB = 0;
        for (auto it = m_dirtyBlocks.begin(); it != m_dirtyBlocks.end(); ++it)
        {
            auto& dirtyBlock = *it;
            
            if (dirtyBlock.offsetB <= curOffsetB + curSizeB)
            {
                curSizeB = std::max(curOffsetB + curSizeB, dirtyBlock.offsetB + dirtyBlock.sizeB) - curOffsetB;
            }
            else
            {
                m_glBuffer->SetSubData(curOffsetB, curSizeB, m_data + curOffsetB);
                
                curOffsetB = dirtyBlock.offsetB;
                curSizeB = dirtyBlock.sizeB;
            }
            
            if (it + 1 == m_dirtyBlocks.end())
            {
                m_glBuffer->SetSubData(curOffsetB, curSizeB, m_data + curOffsetB);
            }
        }

        m_dirtyBlocks.clear();
    }
}
