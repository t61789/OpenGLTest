#pragma once
#include "const.h"
#include "string_handle.h"

namespace op
{
    class Shader;
    class DataSet;
    class ITexture;

    class TextureSet
    {
    public:
        TextureSet() = default;
        ~TextureSet();
        TextureSet(const TextureSet& other);
        TextureSet(TextureSet&& other) noexcept = delete;
        TextureSet& operator=(const TextureSet& other) = delete;
        TextureSet& operator=(TextureSet&& other) noexcept = delete;

        void SetTexture(string_hash nameId, crsp<ITexture> texture);
        void RemoveTexture(string_hash nameId);
        sp<ITexture> GetTexture(string_hash nameId);
        void ApplyTextures(Shader* shader);
        void FillFrom(TextureSet* other);
        void UpdateHash();
        size_t GetHash();

    private:
        bool m_hashDirty = true;
        size_t m_hash = 0;
        vec<std::pair<string_hash, sp<ITexture>>> m_textures;
    };
}
