#include "cull_mode.h"

#include <unordered_map>

#include "utils.h"

namespace op
{
    static std::unordered_map<std::string, CullMode> cullModeTable =
    {
        {"None", CullMode::None},
        {"Front", CullMode::Front},
        {"Back", CullMode::Back},
        {"FrontAndBack", CullMode::FrontAndBack}
    };

    CullModeMgr::CullModeMgr()
    {
        glDisable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }

    CullModeMgr::~CullModeMgr()
    {
        glDisable(GL_CULL_FACE);
    }

    void CullModeMgr::SetCullMode(const CullMode cullMode)
    {
        if (curMode == cullMode)
        {
            return;
        }

        if (curMode == CullMode::None && cullMode != CullMode::None)
        {
            glEnable(GL_CULL_FACE);
        }
        else if (curMode != CullMode::None && cullMode == CullMode::None)
        {
            glDisable(GL_CULL_FACE);
        }

        if (cullMode == CullMode::Front)
        {
            glCullFace(GL_FRONT);
        }
        else if (cullMode == CullMode::Back)
        {
            glCullFace(GL_BACK);
        }
        else if (cullMode == CullMode::FrontAndBack)
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
            return CullMode::None;
        }

        return it->second;
    }
}