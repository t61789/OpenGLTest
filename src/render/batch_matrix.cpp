#include "batch_matrix.h"

#include <algorithm>

#include "utils.h"

namespace op
{
    BatchMatrix::BatchMatrix(uint32_t capacity, uint32_t glSlot)
    {
        m_buffer = std::make_unique<StructuredBuffer>(sizeof(Elem), capacity, glSlot);
        m_objectInfos.resize(m_buffer->GetCount());
    }

    uint32_t BatchMatrix::Alloc()
    {
        while (true)
        {
            if (m_nextEmpty >= m_buffer->GetCount())
            {
                Reserve(m_buffer->GetCount() * 2);
            }
            
            auto& info = m_objectInfos[m_nextEmpty++];
            if (!info.enable)
            {
                info.enable = true;
                return m_nextEmpty - 1;
            }
        }
    }

    void BatchMatrix::Release(const uint32_t index)
    {
        auto& info = m_objectInfos[index];
        info.enable = false;
        info.data = nullptr;
        m_nextEmpty = std::min(m_nextEmpty, index);
    }

    void BatchMatrix::Reserve(const uint32_t count)
    {
        if (count <= m_buffer->GetCount())
        {
            return;
        }
        m_buffer->Resize(count);
        m_objectInfos.resize(count);
    }

    void BatchMatrix::SubmitData(const uint32_t index, const Elem* data)
    {
        if (!exists(m_dirtyObjects, index))
        {
            m_dirtyObjects.push_back(index);
        }
        m_objectInfos[index].data = data;
    }

    void BatchMatrix::SubmitDataActually()
    {
        if (m_dirtyObjects.empty())
        {
            return;
        }
        
        for (auto i : m_dirtyObjects)
        {
            auto& info = m_objectInfos[i];
            if (info.data)
            {
                m_buffer->SetData(i, info.data);
                info.data = nullptr;
            }
        }
        m_dirtyObjects.clear();
    }

    void BatchMatrix::Use()
    {
        SubmitDataActually();
        m_buffer->Use();
    }
}
