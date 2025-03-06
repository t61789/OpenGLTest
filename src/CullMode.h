#pragma once
#include <string>

enum CullMode
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
