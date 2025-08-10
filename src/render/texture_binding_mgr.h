#pragma once
#include <unordered_map>
#include <vector>

#include "string_handle.h"
#include "texture.h"

namespace op
{
    class TextureBindingMgr
    {
    public:
        struct BindingInfo
        {
            Texture* texture = nullptr;
            int slot = -1;
        };
        
        TextureBindingMgr();

        std::vector<BindingInfo> BindTextures(const std::vector<Texture*>& textures);

    private:
        struct Slot
        {
            Texture* texture;
            bool hasBound = false;
        };

        std::vector<Slot> m_slots;
        std::unordered_map<Texture*, int> m_boundTextures;
    };
}
