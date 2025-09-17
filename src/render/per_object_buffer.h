#pragma once
#include "common/elem_accessor_fixed.h"
#include "math/matrix4x4.h"

namespace op
{
    class GlSubmitBuffer;

    class PerObjectBuffer
    {
    public:
        struct alignas(16) Elem 
        {
            Matrix4x4 localToWorld;
            Matrix4x4 worldToLocal;
        };
        
        PerObjectBuffer(uint32_t capacity, uint32_t slot);
        
        void Use();
        
        uint32_t Register();
        void UnRegister(uint32_t index);
        void Reserve(uint32_t count);
        void SubmitData(uint32_t index, cr<Elem> data);

    private:
        struct ObjectInfo
        {
            bool enable = false;
        };

        uint32_t m_nextEmpty = 0;
        vec<ObjectInfo> m_objectInfos;
        up<ElemAccessorFixed<Elem>> m_bufferAccessor;

        sp<GlSubmitBuffer> m_submitBuffer;
    };
}
