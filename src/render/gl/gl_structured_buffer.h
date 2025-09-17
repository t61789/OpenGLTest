#pragma once
#include <assert.h>
#include <cstdint>
#include <stdexcept>
#include <glad/glad.h>

#include "const.h"
#include "utils.h"
#include "gl_submit_buffer.h"

namespace op
{
    class GlSubmitBuffer;

    template <typename T>
    class GlStructuredBuffer
    {
    public:
        GlStructuredBuffer(uint32_t capacityT, uint32_t usage, uint32_t slot);
        ~GlStructuredBuffer();

        void Use();
        
        void Reserve(uint32_t capacityT);
        void Resize(uint32_t sizeT);
        void SetCapacity(uint32_t capacityT);
        void Add(T val);
        void Set(uint32_t indexT, cr<T> val);
        T Get(uint32_t indexT);
        
        uint32_t Size() const { return m_sizeT;}
        uint32_t Capacity() const { return m_capacityT;}

    private:
        uint32_t m_capacityT = 0;
        uint32_t m_sizeT = 0;
        uint32_t m_usage = 0;
        uint32_t m_slot = 0;

        up<GlSubmitBuffer> m_buffer;
    };

    template <typename T>
    GlStructuredBuffer<T>::GlStructuredBuffer(const uint32_t capacityT, uint32_t usage, uint32_t slot)
    {
        assert(capacityT > 0);

        m_capacityT = capacityT;
        m_usage = usage;
        m_slot = slot;
        m_sizeT = 0;
        m_buffer = mup<GlSubmitBuffer>(GL_SHADER_STORAGE_BUFFER, usage, slot, capacityT * sizeof(T));
    }

    template <typename T>
    GlStructuredBuffer<T>::~GlStructuredBuffer()
    {
        m_buffer.reset();
    }

    template <typename T>
    void GlStructuredBuffer<T>::Use()
    {
        m_buffer->Use();
    }

    template <typename T>
    void GlStructuredBuffer<T>::Reserve(const uint32_t capacityT)
    {
        if (m_capacityT >= capacityT)
        {
            return;
        }

        SetCapacity(capacityT);
    }

    template <typename T>
    void GlStructuredBuffer<T>::Resize(const uint32_t sizeT)
    {
        Reserve(sizeT);

        m_sizeT = sizeT;
    }

    template <typename T>
    void GlStructuredBuffer<T>::Add(T val)
    {
        assert(m_sizeT <= m_capacityT);
        
        if (m_sizeT == m_capacityT)
        {
            SetCapacity(m_capacityT * 2);
        }

        m_buffer->SetData(m_sizeT * sizeof(T), sizeof(T), &val);
        m_sizeT++;
    }

    template <typename T>
    void GlStructuredBuffer<T>::Set(const uint32_t indexT, cr<T> val)
    {
        assert(indexT <= m_sizeT);

        if (indexT == m_sizeT)
        {
            Add(val);
            return;
        }

        m_buffer->SetData(indexT * sizeof(T), sizeof(T), &val);
    }

    template <typename T>
    T GlStructuredBuffer<T>::Get(const uint32_t indexT)
    {
        if (indexT >= m_sizeT)
        {
            THROW_ERRORF("Index out of range: %d >= %d", indexT, m_sizeT);
        }

        T val;
        m_buffer->GetData(indexT * sizeof(T), sizeof(T), &val);
        return val;
    }

    template <typename T>
    void GlStructuredBuffer<T>::SetCapacity(const uint32_t capacityT)
    {
        assert(capacityT > 0);
        
        m_buffer->Resize(capacityT * sizeof(T));
        m_capacityT = capacityT;
        m_sizeT = std::min(m_sizeT, capacityT);
    }
}
