#pragma once
#include "cbuffer.h"
#include "structured_buffer.h"
#include "math/matrix4x4.h"

namespace op
{
    struct alignas(16) PerObjectStruct 
    {
        Matrix4x4 localToWorld;
        Matrix4x4 worldToLocal;
    };
    
    class PerObjectBuffer
    {
    public:
        PerObjectBuffer();
        ~PerObjectBuffer();
        uint32_t BindObject();
        void UnbindObject(uint32_t index);
        void Expand();
        void SubmitData(uint32_t index, const PerObjectStruct* data);
        void Use(std::optional<uint32_t> index);

    private:
        struct ObjectInfo
        {
            bool enable = false;
            const PerObjectStruct* data = nullptr;
        };

        uint32_t m_nextEmpty = 0;
        StructuredBuffer* m_buffer;
        std::vector<ObjectInfo> m_objectInfos;
        std::vector<uint32_t> m_dirtyObjects;

        CBuffer* m_objectIndexBuffer;
        uint32_t m_objectIndexSubmitBuffer[4];
    };
}
