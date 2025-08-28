#pragma once
#include "cbuffer.h"
#include "structured_buffer.h"
#include "math/matrix4x4.h"

namespace op
{
    
    class BatchMatrix
    {
    public:
        struct alignas(16) Elem 
        {
            Matrix4x4 localToWorld;
            Matrix4x4 worldToLocal;
        };
        
        BatchMatrix(uint32_t capacity, uint32_t glSlot);
        uint32_t Alloc();
        void Release(uint32_t index);
        void Reserve(uint32_t count);
        void SubmitData(uint32_t index, const Elem* data); // 由于是lazy地上传数据，所以需要确保传入的data是持久可读取的，直到对应的元素被Release
        void SubmitDataActually();
        void Use();

    private:
        struct ObjectInfo
        {
            bool enable = false;
            const Elem* data = nullptr;
        };

        uint32_t m_nextEmpty = 0;
        std::unique_ptr<StructuredBuffer> m_buffer;
        std::vector<ObjectInfo> m_objectInfos;
        std::vector<uint32_t> m_dirtyObjects;
    };
}
