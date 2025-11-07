#pragma once
#include <nlohmann/adl_serializer.hpp>

#include "i_resource.h"

namespace op
{
    class Shader;

    struct VariantKeyword
    {
        void EnableKeyword(cr<StringHandle> keyword);
        void DisableKeyword(cr<StringHandle> keyword);
        size_t GetHash() const { return m_hash; }
        
    private:
        size_t m_hash = 0;
        vec<StringHandle> m_keywords;
    };

    class ShaderVariants final : public IResource
    {
    public:
        cr<StringHandle> GetPath() override { return m_path; }

        sp<Shader> GetShader();
        sp<Shader> GetShader(cr<VariantKeyword> keyword);

        static sp<ShaderVariants> LoadFromJson(cr<StringHandle> shaderPath, cr<nlohmann::json> json);
        static sp<ShaderVariants> LoadFromFile(cr<StringHandle> path);

    private:
        StringHandle m_path;
        vec<std::pair<size_t, sp<Shader>>> m_shaders;
    };
}
