#pragma once

#include <string>
#include <spirv_glsl.hpp>

#include "i_resource.h"
#include "common/data_set.h"
#include "render/texture_set.h"
#include "render/gl/gl_shader.h"


namespace op
{
    class ITexture;
    class DataSet;
    class GlShader;
    struct CBufferLayout;
    enum class GlTextureType : uint8_t;

    class Shader final : public IResource
    {
    public:
        struct TextureInfo
        {
            GlTextureType type;
            uint32_t location;
        };

        TextureSet textures;
        umap<string_hash, sp<CBufferLayout>> cbuffers;

        Shader();

        template <typename T>
        void SetVal(size_t nameId, cr<T> value);
        void SetVal(string_hash nameId, const float* value, uint32_t countF);

        void Use();

        cr<StringHandle> GetPath() override { return m_path;}

        static sp<Shader> LoadFromFile(cr<StringHandle> path);
        static sp<Shader> LoadFromFile(cr<std::string> preparedVert, cr<std::string> preparedFrag, cr<StringHandle> glslPath = NOT_A_FILE);
        static sp<Shader> LoadFromSpvBase64(cr<std::string> vert, cr<std::string> frag, cr<StringHandle> path = NOT_A_FILE);
        static sp<Shader> LoadFromSpvBinary(vec<uint32_t> vert, vec<uint32_t> frag, cr<StringHandle> path = NOT_A_FILE);

    private:
        StringHandle m_path;
        
        sp<GlShader> m_glShader;
        sp<DataSet> m_dataSet;
        
        template <class T, class GlSetValueFunc>
        void SetValImp(string_hash nameId, const T& value, GlSetValueFunc&& glSetValue);

        void LoadCBuffer(cr<spirv_cross::ShaderResources> resources);
        void LoadTextures(cr<spirv_cross::CompilerGLSL> compiler, cr<spirv_cross::ShaderResources> resources);
        void CreatePredefinedCBuffer(cr<StringHandle> uniformBufferName);

        static vec<uint32_t> LoadSpvFileData(cr<std::string> absolutePath);
        static void CombineSeparateTextures(spirv_cross::CompilerGLSL& compiler);
    };

    template <>
    inline void Shader::SetVal<int>(const size_t nameId, cr<int> value)
    {
        SetValImp(nameId, value, [this](const string_hash n, cr<int> v)
        {
            m_glShader->SetInt(n, v);
        });
    }

    template <>
    inline void Shader::SetVal<float>(const size_t nameId, cr<float> value)
    {
        SetValImp(nameId, value, [this](const string_hash n, cr<float> v)
        {
            m_glShader->SetFloat(n, v);
        });
    }
    
    template <>
    inline void Shader::SetVal<Vec4>(const size_t nameId, cr<Vec4> value)
    {
        SetValImp(nameId, value, [this](const string_hash n, cr<Vec4> v)
        {
            m_glShader->SetFloat4(n, &v.x);
        });
    }

    template <>
    inline void Shader::SetVal<Matrix4x4>(const size_t nameId, cr<Matrix4x4> value)
    {
        SetValImp(nameId, value, [this](const string_hash n, cr<Matrix4x4> v)
        {
            m_glShader->SetMatrix(n, v.GetReadOnlyData());
        });
    }

    template <typename T, typename GlSetValueFunc>
    void Shader::SetValImp(const string_hash nameId, cr<T> value, GlSetValueFunc&& glSetValue)
    {
        auto uniformInfo = m_glShader->GetUniformInfo(nameId);
        assert(uniformInfo);
        if (uniformInfo == nullptr)
        {
            return;
        }

        if (m_dataSet->TrySetImp(nameId, &value, sizeof(value)))
        {
            glSetValue(uniformInfo->name.Hash(), value);
        }
    }
}
