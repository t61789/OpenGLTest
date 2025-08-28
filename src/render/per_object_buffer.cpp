#include "per_object_buffer.h"

#include "utils.h"

namespace op
{
    PerObjectBuffer::PerObjectBuffer()
    {
        m_buffer = new StructuredBuffer(sizeof(PerObjectStruct), PER_OBJECT_BUFFER_INIT_CAPACITY, PER_OBJECT_BUFFER_BINDING_SLOT);
        INCREF(m_buffer)
        m_objectInfos.resize(PER_OBJECT_BUFFER_INIT_CAPACITY);

        auto layout = new CBufferLayout();
        layout->name = StringHandle("ObjectIndexCBuffer");
        layout->size = 16;
        layout->binding = 3;
        layout->glUsage = GL_STREAM_DRAW;
        auto paramName = StringHandle("_ObjectIndex");
        layout->params = {
            {paramName.Hash(), {paramName, 16, 0, layout->name.Hash()}}
        };

        m_objectIndexBuffer = new CBuffer(layout);
        INCREF(m_objectIndexBuffer)
    }

    PerObjectBuffer::~PerObjectBuffer()
    {
        DECREF(m_buffer)
        DECREF(m_objectIndexBuffer)
    }

    uint32_t PerObjectBuffer::BindObject()
    {
        while (true)
        {
            if (m_nextEmpty >= m_buffer->GetCount())
            {
                Expand();
            }
            
            auto& info = m_objectInfos[m_nextEmpty];
            m_nextEmpty++;
            if (!info.enable)
            {
                info.enable = true;
                return m_nextEmpty - 1;
            }
        }
    }

    void PerObjectBuffer::UnbindObject(const uint32_t index)
    {
        auto& info = m_objectInfos[index];
        info.enable = false;
        info.data = nullptr;
        m_nextEmpty = index;
    }

    void PerObjectBuffer::Expand()
    {
        m_buffer->Resize(m_buffer->GetCount() * 2);
        m_objectInfos.resize(m_buffer->GetCount());
    }

    // 由于是lazy地上传数据，所以需要确保传入的data是持久可读取的，直到对应的Object被Unbind
    void PerObjectBuffer::SubmitData(const uint32_t index, const PerObjectStruct* data)
    {
        if (!exists(m_dirtyObjects, index))
        {
            m_dirtyObjects.push_back(index);
        }
        m_objectInfos[index].data = data;
    }

    void PerObjectBuffer::Use(const std::optional<uint32_t> index)
    {
        if (!m_dirtyObjects.empty())
        {
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

        m_buffer->Use();

        if (index.has_value())
        {
            m_objectIndexSubmitBuffer[0] = index.value();
            m_objectIndexBuffer->StartSync();
            m_objectIndexBuffer->Sync(m_objectIndexSubmitBuffer, 0, sizeof(m_objectIndexSubmitBuffer));
            m_objectIndexBuffer->Use();

            GL_CHECK_ERROR(asdasd)
        }
    }
}
