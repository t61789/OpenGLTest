#include "string_handle.h"

namespace op
{
    StringHandle::StringHandle(const char* str)
    {
        m_str = str;
        m_hash = std::hash<std::string>{}(str);
    }

    StringHandle::StringHandle(const std::string& str)
    {
        m_str = str;
        m_hash = std::hash<std::string>{}(str);
    }
}
