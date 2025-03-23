#include "blend_mode.h"

#include <unordered_map>

#include "utils.h"

namespace op
{
    std::unordered_map<std::string, BlendMode> BlendModeMgr::s_blendModeMap =
    {
        {"None", BlendMode::None},
        {"Blend", BlendMode::Blend},
        {"Add", BlendMode::Add}
    };

    BlendModeMgr::BlendModeMgr()
    {
        glDisable(GL_BLEND);
    }

    BlendModeMgr::~BlendModeMgr()
    {
        glDisable(GL_BLEND);
    }

    void BlendModeMgr::SetBlendMode(BlendMode blendMode)
    {
        if (m_blendMode == blendMode)
        {
            return;
        }

        if (m_blendMode != BlendMode::None && blendMode == BlendMode::None)
        {
            glDisable(GL_BLEND);
        }
        else if (m_blendMode == BlendMode::None && blendMode != BlendMode::None)
        {
            glEnable(GL_BLEND);
        }

        if (blendMode == BlendMode::Blend)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else if (blendMode == BlendMode::Add)
        {
            glBlendFunc(GL_ONE, GL_ONE);
        }

        m_blendMode = blendMode;
    }

    BlendMode BlendModeMgr::FromStr(const std::string& str)
    {
        auto it = s_blendModeMap.find(str);
        if (it != s_blendModeMap.end())
        {
            Utils::LogWarning("未找到BlendMode：" + str + " 重置成None");
            return it->second;
        }

        return BlendMode::None;
    }
}

