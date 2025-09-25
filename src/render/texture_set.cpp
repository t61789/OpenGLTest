#include "texture_set.h"

#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "image.h"
#include "shader.h"
#include "i_texture.h"
#include "gl/gl_state.h"

namespace op
{
    TextureSet::~TextureSet()
    {
        m_textures.clear();
    }

    TextureSet::TextureSet(const TextureSet& other)
    {
        m_textures = other.m_textures;
    }

    void TextureSet::SetTexture(const string_hash nameId, crsp<ITexture> texture)
    {
        insert(m_textures, nameId, texture);

        m_hashDirty = true;
    }

    void TextureSet::RemoveTexture(const string_hash nameId)
    {
        remove(m_textures, nameId);

        m_hashDirty = true;
    }

    sp<ITexture> TextureSet::GetTexture(const string_hash nameId)
    {
        if (auto p = find(m_textures, nameId))
        {
            return *p;
        }

        return nullptr;
    }

    void TextureSet::ApplyTextures(Shader* shader)
    {
        ZoneScoped;
        
        static sl<GlTexture*> textures(MAX_SUPPORT_SLOTS);

        auto tempTextureSet = shader->textures;
        tempTextureSet.FillFrom(this);
        auto globalTextureSet = GetGlobalTextureSet();
        if (globalTextureSet != this)
        {
            tempTextureSet.FillFrom(globalTextureSet);
        }

        textures.Resize(tempTextureSet.m_textures.size());
        for (size_t i = 0; i < textures.Size(); ++i)
        {
            if (auto& texture = tempTextureSet.m_textures[i].second)
            {
                textures[i] = texture->GetGlTexture().get();
            }
            else
            {
                textures[i] = GetBR()->missTex->GetGlTexture().get();
            }
        }
        
        auto& resultSlots = GlState::Ins()->BindTextures(textures);
        for (uint32_t i = 0; i < resultSlots.Size(); ++i)
        {
            shader->SetVal(tempTextureSet.m_textures[i].first, resultSlots[i]);
        }
    }

    void TextureSet::FillFrom(TextureSet* other)
    {
        for (auto& [nameId, texture] : m_textures)
        {
            if (texture)
            {
                continue;
            }

            texture = other->GetTexture(nameId);
        }

        m_hashDirty = true;
    }
    
    void TextureSet::UpdateHash()
    {
        m_hash = 0;
        for (auto& [nameId, texture] : m_textures)
        {
            auto t = texture ? texture : GetBR()->missTex;
            auto pairHash = nameId;
            combine_hash_no_order(pairHash, std::hash<size_t>{}(reinterpret_cast<uintptr_t>(t.get())));
            combine_hash_no_order(m_hash, pairHash);
        }

        m_hashDirty = false;
    }

    size_t TextureSet::GetHash()
    {
        if (!m_hashDirty)
        {
            return m_hash;
        }

        UpdateHash();

        return m_hash;
    }
}
