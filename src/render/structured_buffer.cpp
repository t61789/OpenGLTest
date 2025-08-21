#include "structured_buffer.h"

#include <tracy/Tracy.hpp>

#include "render_context.h"
#include "utils.h"

namespace op
{
    StructuredBuffer::StructuredBuffer(const uint32_t stride, const uint32_t count, const uint32_t binding)
    {
        m_binding = binding;
        m_stride = stride;
        m_count = count;
        m_data = aligned_malloc<uint8_t>(stride * count, 16);
        memset(m_data, 0, stride * count);

        CreateBuffer(m_stride * m_count);
        SyncDataForce();

        GL_CHECK_ERROR(创建SSBO)
    }

    StructuredBuffer::~StructuredBuffer()
    {
        aligned_free(m_data);
        glDeleteBuffers(1, &m_glBuffer);
    }
    
    void StructuredBuffer::Use()
    {
        SyncData();
        GetRS()->BindBufferBase(m_binding, GL_SHADER_STORAGE_BUFFER, m_glBuffer);
    }

    void StructuredBuffer::SetData(const uint32_t index, const void* data)
    {
        memcpy(m_data + index * m_stride, data, m_stride);
        m_dirty.insert(index);
    }

    void StructuredBuffer::Resize(uint32_t count)
    {
        count = std::max(count, m_count);
        if (count == m_count)
        {
            return;
        }

        // 创建新的缓冲区，把旧缓冲区里的数据拷贝过去
        auto newData = aligned_malloc<uint8_t>(m_stride * count, 16);
        memcpy(newData, m_data, m_stride * m_count);

        // 删除旧的缓冲区和Buffer
        glDeleteBuffers(1, &m_glBuffer);
        aligned_free(m_data);

        // 创建新的Buffer，并同步数据
        m_count = count;
        m_data = newData;
        CreateBuffer(m_stride * m_count);
        SyncDataForce();
    }
    
    void StructuredBuffer::SyncData()
    {
        if (m_dirty.empty())
        {
            return;
        }

        ZoneScopedN("Sync Data To SSBO");

        // 超过70%的脏数据，则强制同步
        if (static_cast<float>(m_dirty.size()) / static_cast<float>(m_count) > 0.7f)
        {
            SyncDataForce();
            return;
        }

        GetRS()->BindBuffer(GL_SHADER_STORAGE_BUFFER, m_glBuffer);
        auto dstPtr = static_cast<uint8_t*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY));

        uint32_t start = ~0u;
        uint32_t cur = 0u;
        size_t count = 0;
        for (auto index : m_dirty)
        {
            count++;
            
            if (start == ~0u)
            {
                start = index;
                cur = start;
            }

            if (index - cur > 1)
            {
                auto offset = start * m_stride;
                auto size = (cur - start + 1) * m_stride;
                memcpy(dstPtr + offset, m_data + offset, size);
                start = index;
            }
            
            cur = index;

            if (count == m_dirty.size())
            {
                auto offset = start * m_stride;
                auto size = (cur - start + 1) * m_stride;
                memcpy(dstPtr + offset, m_data + offset, size);
            }
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        m_dirty.clear();
    }

    void StructuredBuffer::SyncDataForce()
    {
        GetRS()->BindBuffer(GL_SHADER_STORAGE_BUFFER, m_glBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_stride * m_count, m_data, GL_DYNAMIC_DRAW);

        m_dirty.clear();
    }
    
    void StructuredBuffer::CreateBuffer(const uint32_t size)
    {
        glGenBuffers(1, &m_glBuffer);
        GetRS()->BindBuffer(GL_SHADER_STORAGE_BUFFER, m_glBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }
}
