#pragma once
#include <string>

class StringHandle
{
public:
    StringHandle(const std::string& str);

    size_t GetHash() const;

    const std::string& GetString() const;

    const char* GetCStr() const;

    bool operator==(const StringHandle& other) const;

private:
    std::string m_str;
    size_t m_hash;
};
