#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace op
{
    template <typename T>
    struct SimpleList
    {
        static_assert(std::is_trivial_v<T>);

        SimpleList() = default;
        explicit SimpleList(uint32_t capacity);
        ~SimpleList();
        SimpleList(const SimpleList& other);
        SimpleList(SimpleList&& other) noexcept;
        SimpleList& operator=(const SimpleList& other);
        SimpleList& operator=(SimpleList&& other) noexcept;
        
        T& operator[](size_t index) { return m_data[index]; }
        const T& operator[](size_t index) const { return m_data[index]; }

        T* Data() const { return m_data; }
        uint32_t Size() const { return m_size; }
        uint32_t Capacity() const { return m_capacity; }
        bool Empty() const { return m_size == 0; }
        
        template <bool Check = true>
        void Add(const T& element);
        bool Pop(T& element);
        
        void Resize(uint32_t newSize);
        void Reserve(uint32_t newCapacity);
        void Clear();

    private:
        T* m_data = nullptr;
        uint32_t m_size = 0;
        uint32_t m_capacity = 0;
    };

    template <typename T>
    SimpleList<T>::SimpleList(const uint32_t capacity)
    {
        m_data = new T[capacity];
        m_capacity = capacity;
        m_size = 0;
    }

    template <typename T>
    SimpleList<T>::~SimpleList()
    {
        delete[] m_data;
        m_size = 0;
        m_capacity = 0;
    }

    template <typename T>
    SimpleList<T>::SimpleList(const SimpleList& other)
    {
        if (other.m_capacity != 0)
        {
            m_data = new T[other.m_capacity];
            memcpy(m_data, other.m_data, other.m_capacity * sizeof(T));
        }
        m_capacity = other.m_capacity;
        m_size = other.m_size;
    }

    template <typename T>
    SimpleList<T>::SimpleList(SimpleList&& other) noexcept
    {
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    template <typename T>
    SimpleList<T>& SimpleList<T>::operator=(const SimpleList& other)
    {
        if (this == &other)
        {
            return *this;
        }

        delete[] m_data;
        m_data = nullptr;
        
        if (other.m_capacity != 0)
        {
            m_data = new T[other.m_capacity];
            memcpy(m_data, other.m_data, other.m_capacity * sizeof(T));
        }
        
        m_capacity = other.m_capacity;
        m_size = other.m_size;

        return *this;
    }

    template <typename T>
    SimpleList<T>& SimpleList<T>::operator=(SimpleList&& other) noexcept
    {
        delete[] m_data;
        
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;

        return *this;
    }

    template <typename T>
    template <bool Check>
    void SimpleList<T>::Add(const T& element)
    {
        if constexpr (Check)
        {
            if (m_size + 1 > m_capacity)
            {
                Reserve(std::max(m_capacity * 2, m_size + 1));
            }
        }
        
        m_data[m_size] = element;
        m_size++;
    }

    template <typename T>
    bool SimpleList<T>::Pop(T& element)
    {
        if (m_size == 0)
        {
            return false;
        }

        element = m_data[m_size - 1];
        m_size--;

        return true;
    }

    template <typename T>
    void SimpleList<T>::Resize(const uint32_t newSize)
    {
        Reserve(newSize);

        m_size = newSize;
    }

    template <typename T>
    void SimpleList<T>::Reserve(const uint32_t newCapacity)
    {
        if (newCapacity <= m_capacity)
        {
            return;
        }

        auto newData = new T[newCapacity];
        if (m_data)
        {
            memcpy(newData, m_data, m_size * sizeof(T));
            delete[] m_data;
        }
        m_data = newData;
        m_capacity = newCapacity;
    }

    template <typename T>
    void SimpleList<T>::Clear()
    {
        m_size = 0;
    }
}
