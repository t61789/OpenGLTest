#pragma once
#include "const.h"
#include "render/cbuffer.h"

#include "utils.h"

namespace op
{
    struct Matrix4x4;
    struct Vec4;
    class GlSubmitBuffer;
    class ElemAccessorVar;
    
    class GlCbuffer
    {
    public:
        explicit GlCbuffer(crsp<CBufferLayout> layout);
        ~GlCbuffer();
        GlCbuffer(const GlCbuffer& other) = delete;
        GlCbuffer(GlCbuffer&& other) noexcept = delete;
        GlCbuffer& operator=(const GlCbuffer& other) = delete;
        GlCbuffer& operator=(GlCbuffer&& other) noexcept = delete;

        void Bind();
        void BindBase();
        void Submit();
        
        template <typename T>
        bool Set(size_t nameId, cr<T> val);
        bool Set(size_t nameId, const float* val, size_t count);
        
        template <typename T>
        T Get(size_t nameId);
        void Get(size_t nameId, float* val, size_t count);
        
        template <typename T>
        bool TryGet(size_t nameId, T& val);
        bool TryGet(size_t nameId, float* val, size_t count);

        bool TrySetRaw(string_hash nameId, const void* data, uint32_t sizeB);
        bool TryGetRaw(string_hash nameId, void* data, uint32_t sizeB);

    private:
        sp<CBufferLayout> m_layout;
        sp<GlSubmitBuffer> m_buffer;
    };

    template <typename T>
    bool GlCbuffer::Set(const size_t nameId, cr<T> val)
    {
        return TrySetRaw(nameId, &val, sizeof(T));
    }
    
    template <typename T>
    T GlCbuffer::Get(const size_t nameId)
    {
        T val;
        if (TryGet(nameId, &val, sizeof(T)))
        {
            THROW_ERROR("Failed to get cbuffer param")
        }
        return val;
    }

    template <typename T>
    bool GlCbuffer::TryGet(const size_t nameId, T& val)
    {
        return TryGetRaw(nameId, &val, sizeof(T));
    }
}
