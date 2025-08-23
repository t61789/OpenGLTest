#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "event.h"

namespace op
{
    typedef size_t ManagedBlockId;
    
    class ManagedBuffer
    {
    public:
        explicit ManagedBuffer(uint32_t capacity);
        ~ManagedBuffer();

        void Reserve(uint32_t capacity);
        ManagedBlockId Alloc(uint32_t size);
        void Release(ManagedBlockId id);
        void* GetWriteDst(ManagedBlockId id);
        void GetBlockInfo(ManagedBlockId id, uint32_t& offset, uint32_t& size);
        bool Compaction();

        uint32_t Size() const { return m_size;}
        uint32_t RealSize() const { return m_realSize;}
        uint32_t Capacity() const { return m_capacity;}
        uint8_t* Data() const { return m_data;}

    private:
        struct ManagedBlock
        {
            ManagedBlockId id = ~0u;
            bool enable = true;
            uint32_t offset;
            uint32_t size;
        };
    
        uint32_t m_capacity = 0;
        uint32_t m_size = 0;
        uint32_t m_realSize = 0;
        uint8_t* m_data = nullptr;
        std::vector<ManagedBlock> m_blocks;
        std::unordered_map<ManagedBlockId, size_t> m_blockIndices;

        void Expansion(uint32_t size);
        void SortBlocks();
    };
}
