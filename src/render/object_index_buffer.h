#pragma once
#include "cbuffer.h"

namespace op
{
    class ObjectIndexBuffer
    {
    public:
        ObjectIndexBuffer();
        ~ObjectIndexBuffer();

        void SetIndex(uint32_t index);

    private:
        CBuffer* m_buffer;
    };
}
