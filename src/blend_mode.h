#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace op
{
    enum class BlendMode : uint8_t
    {
        None,
        Blend,
        Add
    };
    
    class BlendModeMgr
    {
    public:
        BlendModeMgr();
        ~BlendModeMgr();
        
        void SetBlendMode(BlendMode blendMode);

        static BlendMode FromStr(const std::string& str);

    private:
        BlendMode m_blendMode = BlendMode::None;
        
        static std::unordered_map<std::string, BlendMode> s_blendModeMap;
    };
}
