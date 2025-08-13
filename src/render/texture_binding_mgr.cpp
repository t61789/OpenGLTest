#include "texture_binding_mgr.h"

#include "const.h"
#include "utils.h"

namespace op
{
    TextureBindingMgr::TextureBindingMgr()
    {
        for (int i = 0; i < TEXTURE_SLOT_LIMIT; i++)
        {
            m_slots.emplace_back();
        }
    }

    std::vector<TextureBindingMgr::BindingInfo> TextureBindingMgr::BindTextures(const std::vector<Texture*>& textures)
    {
        for (auto& slot : m_slots)
        {
            slot.hasBound = false;
        }

        auto needBindingInfos = std::vector<BindingInfo>(textures.size());
        for (size_t i = 0; i < textures.size(); i++)
        {
            auto texture = textures[i];
            if (!texture)
            {
                continue;
            }

            auto it = m_boundTextures.find(texture);
            if (it != m_boundTextures.end())
            {
                // 找到绑定了当前texture的slot
                m_slots[it->second].hasBound = true;
                needBindingInfos[i] = { texture, it->second };
            }
            else
            {
                // 没有任何slot绑定了当前texture，就把slot设置为-1
                needBindingInfos[i] = { texture, -1 };
            }
        }

        auto slot = 5; // TODO 临时使用5，避免和旧材质重复
        for (auto& needBindingInfo : needBindingInfos)
        {
            if (needBindingInfo.slot != -1 || !needBindingInfo.texture || !needBindingInfo.texture->isCreated)
            {
                continue;
            }

            // 找到下一个可用的slot
            while (m_slots[slot].hasBound)
            {
                slot++;
            }

            // 绑定texture
            glActiveTexture(GL_TEXTURE0 + slot);
            if(needBindingInfo.texture->isCubeMap)
            {
                glBindTexture(GL_TEXTURE_CUBE_MAP, needBindingInfo.texture->glTextureId);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, needBindingInfo.texture->glTextureId);
            }

            // 移除之前在这个slot里绑定的texture
            if (m_slots[slot].texture)
            {
                m_boundTextures.erase(m_slots[slot].texture);
            }
            // 添加当前texture，并配置slot
            m_boundTextures[needBindingInfo.texture] = slot;
            needBindingInfo.slot = slot;
            slot++;
        }

        return needBindingInfos;
    }
}
