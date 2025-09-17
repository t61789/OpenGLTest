#pragma once
#include "const.h"
#include "i_memory_block.h"
#include "utils.h"

namespace op
{
    class ElemAccessorVar
    {
    public:
        explicit ElemAccessorVar(crsp<IMemoryBlock> memoryBlock);

        size_t Alloc(uint32_t sizeB);
        void Set(size_t key, const void* data);
        void Get(size_t key, void* data);
        void GetInfo(size_t key, uint32_t& offsetB, uint32_t& sizeB);
        void Remove(size_t key);
        void Compaction();
        void Reserve(uint32_t capacityB);

        uint32_t Size() const { return m_sizeB;}
        uint32_t Capacity() const { return m_capacityB;}

    private:
        struct Element
        {
            bool enable;
            size_t key;
            uint32_t offsetB;
            uint32_t sizeB;
        };
        
        uint32_t m_capacityB = 0;
        uint32_t m_sizeB = 0;
        uint32_t m_realSizeB = 0;

        umap<size_t, uint32_t> m_keyMapper;
        vec<Element> m_elements;
        
        sp<IMemoryBlock> m_memoryBlock;

        void SortElements();
    };
}
