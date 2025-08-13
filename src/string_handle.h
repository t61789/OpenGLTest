#pragma once
#include <string>

namespace op
{
    class StringHandle
    {
    public:
        StringHandle() = default;
        
        StringHandle(const std::string& str);

        size_t Hash() const { return m_hash; }

        const std::string& Str() const { return m_str; }

        const char* CStr() const { return m_str.c_str(); }

        bool operator==(const StringHandle& other) const { return m_hash == other.m_hash; }

        bool operator!=(const StringHandle& other) const { return m_hash != other.m_hash; }

        bool operator==(const std::string& other) const { return m_str == other; }

        bool operator!=(const std::string& other) const { return m_str != other; }

        operator size_t() const { return m_hash; }

        operator const std::string&() const { return m_str; }

    private:
        std::string m_str;
        size_t m_hash;
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
