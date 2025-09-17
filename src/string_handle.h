#pragma once
#include <string>

namespace op
{
    typedef size_t string_hash;
    
    class StringHandle
    {
    public:
        StringHandle();
        StringHandle(const char* str);
        StringHandle(const std::string& str);

        size_t Hash() const { return m_hash; }

        const std::string& Str() const { return m_str; }

        const char* CStr() const { return m_str.c_str(); }

        bool Empty() const;

        bool operator==(const StringHandle& other) const { return m_hash == other.m_hash; }
        
        bool operator==(const std::string& other) const { return m_str == other; }

        bool operator!=(const StringHandle& other) const { return m_hash != other.m_hash; }

        bool operator!=(const std::string& other) const { return m_str != other; }

        operator size_t() const { return m_hash; }

        operator const std::string&() const { return m_str; }

    private:
        std::string m_str;
        size_t m_hash = 0;
    };
}

template<>
struct std::hash<op::StringHandle>
{
    size_t operator()(const op::StringHandle& key) const noexcept
    {
        return key.Hash();
    }
};
