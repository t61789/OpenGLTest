#include "managed_buffer.h"

#include <algorithm>
#include <cstring>

#include "utils.h"

namespace op
{
    ManagedBuffer::ManagedBuffer(const uint32_t capacity)
    {
        m_data = new uint8_t[capacity];
        m_capacity = capacity;
    }

    ManagedBuffer::~ManagedBuffer()
    {
        delete[] m_data;
    }

    void ManagedBuffer::Reserve(const uint32_t capacity)
    {
        if (m_capacity >= capacity)
        {
            return;
        }

        auto newData = new uint8_t[capacity];
        memcpy(newData, m_data, m_capacity);

        delete[] m_data;
        m_data = newData;
        m_capacity = capacity;
    }

    ManagedBlockId ManagedBuffer::Alloc(const uint32_t size)
    {
        Expansion(size);
        
        ManagedBlock block = {get_random_size_t(), true, m_size - size, size};
        m_blocks.push_back(block);
        m_blockIndices[block.id] = m_blocks.size() - 1;

        return block.id;
    }

    void ManagedBuffer::Release(const ManagedBlockId id)
    {
        auto blockIndex = m_blockIndices[id];
        auto& block = m_blocks[blockIndex];
        if (block.offset + block.size == m_size)
        {
            m_size -= block.size;
        }
        m_realSize -= block.size;
        block.enable = false;
    }

    void* ManagedBuffer::GetWriteDst(const ManagedBlockId id)
    {
        return m_data + m_blocks[m_blockIndices[id]].offset;
    }

    void ManagedBuffer::GetBlockInfo(const ManagedBlockId id, uint32_t& offset, uint32_t& size)
    {
        auto& info = m_blocks[m_blockIndices[id]];
        offset = info.offset;
        size = info.size;
    }

    bool ManagedBuffer::Compaction()
    {
        if (m_blocks.empty() || m_size == m_realSize)
        {
            return false;
        }

        SortBlocks();
        
        auto maxBlockSize = max_element(m_blocks, &ManagedBlock::size).size;
        auto tempBlock = new uint8_t[maxBlockSize];

        m_size = 0;
        for (auto& block : m_blocks)
        {
            if (block.offset - m_size > 0)
            {
                memcpy(tempBlock, m_data + block.offset, block.size);
                memcpy(m_data + m_size, tempBlock, block.size);
            }
                
            block.offset = m_size;
            m_size += block.size;
        }
        m_realSize = m_size;

        delete[] tempBlock;

        return true;
    }

    void ManagedBuffer::Expansion(const uint32_t size)
    {
        auto newSize = m_size + size;
        if (newSize > m_capacity)
        {    
            auto newCapacity = m_capacity;
            while (newSize > newCapacity)
            {
                newCapacity *= 2;
            }

            Reserve(newCapacity);
        }

        m_size += size;
        m_realSize += size;
    }
    
    void ManagedBuffer::SortBlocks()
    {
        std::sort(m_blocks.begin(), m_blocks.end(), [](const ManagedBlock& a, const ManagedBlock& b)
        {
            if (a.enable == b.enable)
            {
                return a.offset < b.offset;
            }

            return a.enable > b.enable;
        });
        
        uint32_t disabledCount = 0;
        for (auto it = m_blocks.end() - 1; it != m_blocks.begin(); --it)
        {
            if (it->enable)
            {
                break;
            }

            disabledCount++;
        }

        m_blocks.resize(m_blocks.size() - disabledCount);

        m_blockIndices.clear();
        auto blockIndex = 0;
        for (auto& block : m_blocks)
        {
            m_blockIndices[block.id] = blockIndex++;
        }
    }
}
