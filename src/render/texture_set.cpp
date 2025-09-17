#include "texture_set.h"

#include "game_resource.h"
#include "shader.h"
#include "i_texture.h"
#include "gl/gl_state.h"

namespace op
{
    TextureSet::~TextureSet()
    {
        m_textures.clear();
    }

    void TextureSet::SetTexture(const string_hash nameId, crsp<ITexture> texture)
    {
        m_textures[nameId] = texture;
    }

    void TextureSet::RemoveTexture(const string_hash nameId)
    {
        auto it = m_textures.find(nameId);
        if (it != m_textures.end())
        {
            m_textures.erase(it);
        }
    }

    sp<ITexture> TextureSet::GetTexture(const string_hash nameId)
    {
        auto it = m_textures.find(nameId);
        return it != m_textures.end() ? it->second : nullptr;
    }

    void TextureSet::ApplyTextures(Shader* shader)
    {
        auto globalTextureSet = GetGlobalTextureSet();
        if (globalTextureSet == this)
        {
            return;
        }

        static vecsp<GlTexture> textures;
        static vec<string_hash> textureNames;
        for (auto& [textureNameId, textureInfo] : shader->textures)
        {
            auto localIt = m_textures.find(textureNameId);
            if (localIt != m_textures.end())
            {
                textures.push_back(localIt->second->GetGlTexture());
                textureNames.push_back(textureNameId);
                continue;
            }

            auto globalIt = globalTextureSet->m_textures.find(textureNameId);
            if (globalIt != globalTextureSet->m_textures.end())
            {
                textures.push_back(globalIt->second->GetGlTexture());
                textureNames.push_back(textureNameId);
            }
        }

        auto& resultSlots = GlState::Ins()->BindTextures(textures);
        for (uint32_t i = 0; i < resultSlots.size(); ++i)
        {
            shader->SetVal(textureNames[i], resultSlots[i]);
        }
        
        textures.clear();
        textureNames.clear();
    }
}
