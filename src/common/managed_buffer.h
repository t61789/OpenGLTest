#pragma once
#include <cstdint>

#include "math/vec.h"

namespace op
{
    template <typename T>
    class ManagedBuffer;
    
    template <typename T>
    struct MBPtr
    {
        MBPtr() = default;
        explicit MBPtr(uint32_t index, ManagedBuffer<T>* buffer);

        bool IsEnable() const { return m_enable; }

        T* operator->();
        T& operator*();

    private:
        bool m_enable = false;
        uint32_t m_index = 0;
        ManagedBuffer<T>* m_buffer = nullptr;

        friend class ManagedBuffer<T>;
    };


    
    
    template <typename T>
    class ManagedBuffer
    {
    public:
        MBPtr<T> Alloc();
        void Free(MBPtr<T>& access);

        T* GetData(uint32_t index);

        T& operator[](uint32_t index);

        template <typename F>
        void Foreach(F&& func);

    private:
        struct ElemInfo
        {
            uint32_t empty = true;
        };
        
        vec<T> m_data;
        vec<ElemInfo> m_info;
        uint32_t m_realSizeT = 0;
        uint32_t m_nextEmpty = 0;
    };


    

    template <typename T>
    MBPtr<T>::MBPtr(const uint32_t index, ManagedBuffer<T>* buffer)
    {
        m_index = index;
        m_buffer = buffer;
        m_enable = true;
    }

    template <typename T>
    T* MBPtr<T>::operator->()
    {
        assert(m_enable);
        
        return m_buffer->GetData(m_index);
    }

    template <typename T>
    T& MBPtr<T>::operator*()
    {
        assert(m_enable);
        
        return *m_buffer->GetData(m_index);
    }


    template <typename T>
    MBPtr<T> ManagedBuffer<T>::Alloc()
    {
        for (auto searchIndex = m_nextEmpty; searchIndex <= m_data.size(); ++searchIndex)
        {
            if (searchIndex == m_data.size())
            {
                m_data.push_back({});
                m_info.push_back({});
            }
            
            if (m_info[searchIndex].empty)
            {
                m_realSizeT++;
                m_nextEmpty = searchIndex + 1;
                m_info[searchIndex].empty = false;
                return MBPtr<T>(searchIndex, this);
            }
        }

        throw "Wtf!!!";
    }

    template <typename T>
    void ManagedBuffer<T>::Free(MBPtr<T>& access)
    {
        assert(!m_info[access.m_index].empty);

        access.m_enable = false;
        m_info[access.m_index].empty = true;
        m_nextEmpty = std::min(m_nextEmpty, access.m_index);
        m_realSizeT--;
    }

    template <typename T>
    T* ManagedBuffer<T>::GetData(uint32_t index)
    {
        assert(!m_info[index].empty);
        
        return &m_data[index];
    }

    template <typename T>
    T& ManagedBuffer<T>::operator[](const uint32_t index)
    {
        assert(!m_info[index].empty);
        
        return m_data[index];
    }

    template <typename T>
    template <typename F>
    void ManagedBuffer<T>::Foreach(F&& func)
    {
        for (uint32_t i = 0; i < m_data.size(); ++i)
        {
            if (!m_info[i].empty)
            {
                func(m_data[i]);
            }
        }
    }
}
