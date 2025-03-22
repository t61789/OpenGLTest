#pragma once
#include <string>

namespace op
{
    enum CullMode : uint8_t
    {
        None,
        Front,
        Back,
        FrontAndBack
    };

    class CullModeMgr
    {
    public:
        CullMode curMode = None;

        CullModeMgr();
        ~CullModeMgr();
        
        void SetCullMode(CullMode cullMode);
        
        static CullMode FromStr(const std::string& str);
    };
}
