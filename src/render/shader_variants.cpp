#include "shader_variants.h"

#include "game_resource.h"
#include "shader.h"

namespace op
{
    void VariantKeyword::EnableKeyword(cr<StringHandle> keyword)
    {
        assert(keyword.Hash() != 0);
        assert(!find_if(m_keywords, [&keyword](cr<StringHandle> k){ return keyword.Hash() == k.Hash(); }));

        combine_hash_no_order(m_hash, keyword.Hash());
        m_keywords.push_back(keyword);
    }

    void VariantKeyword::DisableKeyword(cr<StringHandle> keyword)
    {
        assert(keyword.Hash() != 0);
        
        remove_if(m_keywords, [&keyword](cr<StringHandle> k){ return keyword.Hash() == k.Hash(); });

        m_hash = 0;
        for (auto& k : m_keywords)
        {
            combine_hash_no_order(m_hash, k.Hash());
        }
    }

    sp<Shader> ShaderVariants::GetShader()
    {
        return m_shaders[0].second;
    }

    sp<Shader> ShaderVariants::GetShader(cr<VariantKeyword> keyword)
    {
        auto pair = find(m_shaders, keyword.GetHash());
        if (!pair)
        {
            return nullptr;
        }

        return *pair;
    }

    sp<ShaderVariants> ShaderVariants::LoadFromJson(cr<StringHandle> shaderPath, cr<nlohmann::json> json)
    {
        assert(GetGR()->GetResource(shaderPath) == nullptr);
        
        auto variants = msp<ShaderVariants>();
        variants->m_path = shaderPath;

        for (auto& variantJson : json)
        {
            auto variantKeyword = VariantKeyword();
            for (auto& keyword : variantJson["variant"])
            {
                variantKeyword.EnableKeyword(StringHandle(keyword.get<std::string>()));
            }
            assert(!find(variants->m_shaders, variantKeyword.GetHash()));
            
            const auto& shader = Shader::LoadFromSpvBase64(
                variantJson["vert"].get<std::string>(),
                variantJson["frag"].get<std::string>(),
                shaderPath);
            shader->m_variants = variants;

            variants->m_shaders.emplace_back(variantKeyword.GetHash(), shader);
        }

        GetGR()->RegisterResource(shaderPath, variants);
        
        log_info("Load shader variants: %s", shaderPath.CStr());

        return variants;
    }

    sp<ShaderVariants> ShaderVariants::LoadFromFile(cr<StringHandle> path)
    {
        if(auto result = GetGR()->GetResource<ShaderVariants>(path))
        {
            return result;
        }
        
        THROW_ERRORF("Shader未在pack中找到：%s", path.CStr())
    }
}
