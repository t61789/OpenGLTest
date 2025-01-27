#include "CullMode.h"

#include <unordered_map>

#include "Utils.h"

std::unordered_map<std::string, CullMode> cullModeTable =
{
    {"None", None},
    {"Front", Front},
    {"Back", Back},
    {"FrontAndBack", FrontAndBack}
};

CullModeMgr::CullModeMgr()
{
    glDisable(GL_CULL_FACE);
}

CullModeMgr::~CullModeMgr()
{
    glDisable(GL_CULL_FACE);
}

void CullModeMgr::setCullMode(const CullMode cullMode)
{
    if (curMode == cullMode)
    {
        return;
    }

    if (curMode == None && cullMode != None)
    {
        glEnable(GL_CULL_FACE);
    }
    else if (curMode != None && cullMode == None)
    {
        glDisable(GL_CULL_FACE);
    }

    if (cullMode == Front)
    {
        glCullFace(GL_FRONT);
    }
    else if (cullMode == Back)
    {
        glCullFace(GL_BACK);
    }
    else if (cullMode == FrontAndBack)
    {
        glCullFace(GL_FRONT_AND_BACK);
    }

    curMode = cullMode;
}

CullMode CullModeMgr::FromStr(const std::string& str)
{
    auto it = cullModeTable.find(str);
    if (it == cullModeTable.end())
    {
        Utils::LogWarning("未找到CullMode：" + str + " 重置成None");
        return None;
    }

    return it->second;
}
