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
    
    void setCullMode(CullMode cullMode);
    
    static CullMode FromStr(const std::string& str);
};
