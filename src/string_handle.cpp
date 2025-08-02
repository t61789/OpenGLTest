#include "string_handle.h"

StringHandle::StringHandle(const std::string& str)
{
    m_str = str;
    m_hash = std::hash<std::string>{}(str);
}

size_t StringHandle::GetHash() const
{
    return m_hash;
}

const std::string& StringHandle::GetString() const
{
    return m_str;
}

const char* StringHandle::GetCStr() const
{
    return m_str.c_str();
}

bool StringHandle::operator==(const StringHandle& other) const
{
    return m_hash == other.m_hash;
}


