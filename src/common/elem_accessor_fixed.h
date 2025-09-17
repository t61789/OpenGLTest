#pragma once
#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "const.h"
#include "utils.h"
#include "i_memory_block.h"

namespace op
{
    template <typename T>
    class ElemAccessorFixed
    {
    public:
        explicit ElemAccessorFixed(crsp<IMemoryBlock> memoryBlock);
        ~ElemAccessorFixed();
        ElemAccessorFixed(const ElemAccessorFixed& other) = delete;
        ElemAccessorFixed(ElemAccessorFixed&& other) noexcept = delete;
        ElemAccessorFixed& operator=(const ElemAccessorFixed& other) = delete;
        ElemAccessorFixed& operator=(ElemAccessorFixed&& other) noexcept = delete;

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

        sp<IMemoryBlock> m_memoryBlock;
    };

    template <typename T>
    ElemAccessorFixed<T>::ElemAccessorFixed(crsp<IMemoryBlock> memoryBlock)
    {
        assert(memoryBlock->Size() % sizeof(T) == 0);

        m_capacityT = memoryBlock->Size() / sizeof(T);
        m_sizeT = 0;
        m_memoryBlock = memoryBlock;
    }

    template <typename T>
    ElemAccessorFixed<T>::~ElemAccessorFixed()
    {
        m_memoryBlock.reset();
    }

    template <typename T>
    void ElemAccessorFixed<T>::Reserve(const uint32_t capacityT)
    {
        if (m_capacityT >= capacityT)
        {
            return;
        }

        SetCapacity(capacityT);
    }

    template <typename T>
    void ElemAccessorFixed<T>::Resize(const uint32_t sizeT)
    {
        Reserve(sizeT);

        m_sizeT = sizeT;
    }

    template <typename T>
    void ElemAccessorFixed<T>::Add(T val)
    {
        assert(m_sizeT <= m_capacityT);
        
        if (m_sizeT == m_capacityT)
        {
            SetCapacity(m_capacityT * 2);
        }

        m_memoryBlock->SetData(m_sizeT * sizeof(T), sizeof(T), &val);
        m_sizeT++;
    }

    template <typename T>
    void ElemAccessorFixed<T>::Set(const uint32_t indexT, cr<T> val)
    {
        assert(indexT <= m_sizeT);

        if (indexT == m_sizeT)
        {
            Add(val);
            return;
        }

        m_memoryBlock->SetData(indexT * sizeof(T), sizeof(T), &val);
    }

    template <typename T>
    T ElemAccessorFixed<T>::Get(const uint32_t indexT)
    {
        if (indexT >= m_sizeT)
        {
            THROW_ERRORF("Index out of range: %d >= %d", indexT, m_sizeT);
        }

        T val;
        m_memoryBlock->GetData(indexT * sizeof(T), sizeof(T), &val);
        return val;
    }

    template <typename T>
    void ElemAccessorFixed<T>::SetCapacity(const uint32_t capacityT)
    {
        assert(capacityT > 0);
        
        m_memoryBlock->Resize(capacityT * sizeof(T));
        m_capacityT = capacityT;
        m_sizeT = std::min(m_sizeT, capacityT);
    }
}
