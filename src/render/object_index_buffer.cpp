#include "object_index_buffer.h"

namespace op
{
    ObjectIndexBuffer::ObjectIndexBuffer()
    {
        auto layout = new CBufferLayout();
        layout->name = StringHandle("ObjectIndexBuffer"),
        layout->size = sizeof(uint32_t);
        layout->binding = 2;
        layout->glUsage = GL_STREAM_DRAW;

        auto paramName = StringHandle("index");
        layout->params = {
            {paramName.Hash(), {paramName, sizeof(uint32_t), 0, layout.name.Hash()}}
        };

        m_buffer = new CBuffer(layout);
        INCREF(m_buffer);
    }

    ObjectIndexBuffer::~ObjectIndexBuffer()
    {
        DECREF(m_buffer);
    }

    
}
