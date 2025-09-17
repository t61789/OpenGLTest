#include "elem_accessor_var.h"

namespace op
{
    ElemAccessorVar::ElemAccessorVar(crsp<IMemoryBlock> memoryBlock)
    {
        m_memoryBlock = memoryBlock;
        m_capacityB = memoryBlock->Size();
        m_sizeB = m_realSizeB = 0;
    }

    size_t ElemAccessorVar::Alloc(const uint32_t sizeB)
    {
        auto newSize = m_sizeB + sizeB;
        if (newSize > m_capacityB)
        {
            auto newCapacityB = m_capacityB;
            while (newSize > newCapacityB)
            {
                newCapacityB *= 2;
            }

            m_memoryBlock->Resize(newCapacityB);
            m_capacityB = newCapacityB;
        }

        Element element;
        element.key = get_random_size_t();
        element.offsetB = m_sizeB;
        element.sizeB = sizeB;

        assert(m_keyMapper.find(element.key) == m_keyMapper.end());

        m_keyMapper[element.key] = static_cast<uint32_t>(m_elements.size());
        m_elements.push_back(element);

        m_sizeB = newSize;
        m_realSizeB += sizeB;

        return element.key;
    }

    void ElemAccessorVar::Set(const size_t key, const void* data)
    {
        auto it = m_keyMapper.find(key);
        if (it == m_keyMapper.end())
        {
            THROW_ERRORF("Can't find element with index %d", key)
        }
        auto& element = m_elements[it->second];

        m_memoryBlock->SetData(element.offsetB, element.sizeB, data);
    }

    void ElemAccessorVar::Get(const size_t key, void* data)
    {
        auto it = m_keyMapper.find(key);
        if (it == m_keyMapper.end())
        {
            THROW_ERRORF("Can't find element with index %d", key)
        }

        auto& element = m_elements[it->second];
        m_memoryBlock->GetData(element.offsetB, element.sizeB, data);
    }

    void ElemAccessorVar::GetInfo(const size_t key, uint32_t& offsetB, uint32_t& sizeB)
    {
        auto it = m_keyMapper.find(key);
        if (it == m_keyMapper.end())
        {
            THROW_ERRORF("Can't find element with index %d", key)
        }

        auto& element = m_elements[it->second];
        offsetB = element.offsetB;
        sizeB = element.sizeB;
    }

    void ElemAccessorVar::Remove(const size_t key)
    {
        auto it = m_keyMapper.find(key);
        if (it == m_keyMapper.end())
        {
            THROW_ERRORF("Can't find element with index %d", key)
        }

        auto& element = m_elements[it->second];
        if (it->second == m_elements.size() - 1)
        {
            m_sizeB -= element.sizeB;
        }
        element.enable = false;
        m_realSizeB -= element.sizeB;
        m_keyMapper.erase(it);
    }

    void ElemAccessorVar::Compaction()
    {
        if (m_realSizeB == m_sizeB)
        {
            return;
        }

        SortElements();
        
        auto maxBlockSize = max_element(m_elements, &Element::sizeB).sizeB;
        auto tempBlock = new uint8_t[maxBlockSize];

        m_sizeB = 0;
        for (auto& elem : m_elements)
        {
            if (elem.offsetB - m_sizeB > 0)
            {
                m_memoryBlock->GetData(elem.offsetB, elem.sizeB, tempBlock);
                m_memoryBlock->SetData(m_sizeB, elem.sizeB, tempBlock);
            }

            elem.offsetB = m_sizeB;
            m_sizeB += elem.sizeB;
        }
        
        m_realSizeB = m_sizeB;

        delete[] tempBlock;
    }

    void ElemAccessorVar::Reserve(const uint32_t capacityB)
    {
        if (m_capacityB >= capacityB)
        {
            return;
        }

        m_memoryBlock->Resize(capacityB);
    }

    void ElemAccessorVar::SortElements()
    {
        std::sort(m_elements.begin(), m_elements.end(), [](const Element& a, const Element& b)
        {
            if (a.enable == b.enable)
            {
                return a.offsetB < b.offsetB;
            }

            return a.enable > b.enable;
        });
        
        uint32_t disabledCount = 0;
        for (auto it = m_elements.end() - 1; it != m_elements.begin(); --it)
        {
            if (it->enable)
            {
                break;
            }

            disabledCount++;
        }

        m_elements.resize(m_elements.size() - disabledCount);

        m_keyMapper.clear();
        auto index = 0;
        for (auto& elem : m_elements)
        {
            m_keyMapper[elem.key] = index++;
        }
    }
}
