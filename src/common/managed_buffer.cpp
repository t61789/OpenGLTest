#include "managed_buffer.h"

#include <algorithm>
#include <cstring>

#include "utils.h"

namespace op
{
    ManagedBuffer::ManagedBuffer(const uint32_t capacityB)
    {
        m_data = new uint8_t[capacityB];
        m_capacityB = capacityB;
    }

    ManagedBuffer::~ManagedBuffer()
    {
        delete[] m_data;
    }

    void ManagedBuffer::Reserve(const uint32_t capacityB)
    {
        if (m_capacityB >= capacityB)
        {
            return;
        }

        auto newData = new uint8_t[capacityB];
        memcpy(newData, m_data, m_capacityB);

        delete[] m_data;
        m_data = newData;
        m_capacityB = capacityB;
    }

    ManagedBlockId ManagedBuffer::Alloc(const uint32_t sizeB)
    {
        Expansion(sizeB);
        
        ManagedBlock block = {get_random_size_t(), true, m_sizeB - sizeB, sizeB};
        m_blocks.push_back(block);
        m_blockIndices[block.id] = m_blocks.size() - 1;

        return block.id;
    }

    void ManagedBuffer::Release(const ManagedBlockId id)
    {
        auto blockIndex = m_blockIndices[id];
        auto& block = m_blocks[blockIndex];
        if (block.offsetB + block.sizeB == m_sizeB)
        {
            m_sizeB -= block.sizeB;
        }
        m_realSize -= block.sizeB;
        block.enable = false;
    }

    void* ManagedBuffer::GetWriteDst(const ManagedBlockId id)
    {
        return m_data + m_blocks[m_blockIndices[id]].offsetB;
    }

    void ManagedBuffer::GetBlockInfo(const ManagedBlockId id, uint32_t& offsetB, uint32_t& sizeB)
    {
        auto& info = m_blocks[m_blockIndices[id]];
        offsetB = info.offsetB;
        sizeB = info.sizeB;
    }

    bool ManagedBuffer::Compaction()
    {
        if (m_blocks.empty() || m_sizeB == m_realSize)
        {
            return false;
        }

        SortBlocks();
        
        auto maxBlockSize = max_element(m_blocks, &ManagedBlock::sizeB).sizeB;
        auto tempBlock = new uint8_t[maxBlockSize];

        m_sizeB = 0;
        for (auto& block : m_blocks)
        {
            if (block.offsetB - m_sizeB > 0)
            {
                memcpy(tempBlock, m_data + block.offsetB, block.sizeB);
                memcpy(m_data + m_sizeB, tempBlock, block.sizeB);
            }
                
            block.offsetB = m_sizeB;
            m_sizeB += block.sizeB;
        }
        m_realSize = m_sizeB;

        delete[] tempBlock;

        return true;
    }

    void ManagedBuffer::Expansion(const uint32_t size)
    {
        auto newSize = m_sizeB + size;
        if (newSize > m_capacityB)
        {    
            auto newCapacity = m_capacityB;
            while (newSize > newCapacity)
            {
                newCapacity *= 2;
            }

            Reserve(newCapacity);
        }

        m_sizeB += size;
        m_realSize += size;
    }
    
    void ManagedBuffer::SortBlocks()
    {
        std::sort(m_blocks.begin(), m_blocks.end(), [](const ManagedBlock& a, const ManagedBlock& b)
        {
            if (a.enable == b.enable)
            {
                return a.offsetB < b.offsetB;
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
