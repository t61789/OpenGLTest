#include "shader.h"

#include <fstream>
#include <regex>
#include <unordered_set>
#include <utility>

#include "render_texture.h"
#include "string_handle.h"
#include "utils.h"
#include "const.h"
#include "game_resource.h"
#include "common/data_set.h"
#include "render/gl/gl_state.h"

namespace op
{
    using namespace std;

    Shader::Shader()
    {
        m_dataSet = msp<DataSet>();
    }

    void Shader::Use()
    {
        m_glShader->Use();
    }
    
    void Shader::SetVal(const string_hash nameId, const float* value, const uint32_t countF)
    {
        auto uniformInfo = m_glShader->GetUniformInfo(nameId);
        if (uniformInfo == nullptr)
        {
            return;
        }

        if (m_dataSet->TrySetImp(nameId, value, sizeof(value) * countF))
        {
            m_glShader->SetFloatArr(uniformInfo->location, value, countF);
        }
    }

    sp<Shader> Shader::LoadFromSpvBase64(cr<std::string> vert, cr<std::string> frag, cr<StringHandle> path)
    {
        auto vertStr = Utils::Binary8To32(Utils::Base64ToBinary(vert));
        auto fragStr = Utils::Binary8To32(Utils::Base64ToBinary(frag));

        return LoadFromSpvBinary(std::move(vertStr), std::move(fragStr), path);
    }

    sp<Shader> Shader::LoadFromSpvBinary(vec<uint32_t> vert, vec<uint32_t> frag, cr<StringHandle> path)
    {
        spirv_cross::CompilerGLSL::Options options;
        options.version = 460;
        
        spirv_cross::CompilerGLSL vertCompilerGlsl(std::move(vert));
        vertCompilerGlsl.set_common_options(options);
        CombineSeparateTextures(vertCompilerGlsl);
        auto vSource = vertCompilerGlsl.compile();
        spirv_cross::ShaderResources vertShaderResources = vertCompilerGlsl.get_shader_resources();
        
        spirv_cross::CompilerGLSL fragCompilerGlsl(std::move(frag));
        fragCompilerGlsl.set_common_options(options);
        CombineSeparateTextures(fragCompilerGlsl);
        auto fSource = fragCompilerGlsl.compile();
        spirv_cross::ShaderResources fragShaderResources = fragCompilerGlsl.get_shader_resources();
        
        auto result = msp<Shader>();
        try
        {
            result->m_glShader = msp<GlShader>(vSource, fSource);
        }
        catch (const std::exception& e)
        {
            THROW_ERRORF("Shader加载失败：%s \n %s", path.CStr(), e.what())
        }

        // Load CBuffer
        result->LoadCBuffer(vertShaderResources);
        result->LoadCBuffer(fragShaderResources);

        // Load Textures
        result->LoadTextures(vertCompilerGlsl, vertShaderResources);
        result->LoadTextures(fragCompilerGlsl, fragShaderResources);

        return result;
    }

    std::vector<uint32_t> Shader::LoadSpvFileData(const string& absolutePath)
    {
        std::ifstream file(absolutePath.c_str(), std::ios::binary);
        if (!file)
        {
            THROW_ERROR("打开 SPIR-V 文件失败")
        }
        
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (size % sizeof(uint32_t) != 0)
        {
            THROW_ERROR("SPIR-V file 大小不正确")
        }
        
        std::vector<uint32_t> spirv(size / sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(spirv.data()), static_cast<std::streamsize>(size));
        return spirv;
    }

    void Shader::LoadCBuffer(cr<spirv_cross::ShaderResources> resources)
    {
        for (const auto& uniformBuffer : resources.uniform_buffers)
        {
            auto uniformBufferName = StringHandle(uniformBuffer.name);
            
            // 是内置cbuffer的话就加载到GameResource中
            if (GetGR()->IsPredefinedCbuffer(uniformBufferName))
            {
                CreatePredefinedCBuffer(uniformBufferName);
                continue;
            }

            // 不是内置的cbuffer，就由当前Shader持有这个cbuffer
            if (cbuffers.find(uniformBufferName) != cbuffers.end())
            {
                continue;
            }

            cbuffers[uniformBufferName] = msp<CBufferLayout>(m_glShader.get(), uniformBuffer.name.c_str());
        }
    }

    void Shader::LoadTextures(
        cr<spirv_cross::CompilerGLSL> compiler,
        cr<spirv_cross::ShaderResources> resources)
    {
        for (const auto& image : resources.sampled_images)
        {
            const auto& type = compiler.get_type(image.type_id);

            if (type.basetype != spirv_cross::SPIRType::SampledImage)
            {
                continue;
            }

            GlTextureType textureType;
            if (type.image.dim == spv::Dim2D)
            {
                textureType = GlTextureType::TEXTURE_2D;
            }
            else if (type.image.dim == spv::DimCube)
            {
                textureType = GlTextureType::TEXTURE_CUBE_MAP;
            }
            else
            {
                THROW_ERROR("不支持的纹理类型")
            }

            auto nameHash = StringHandle(image.name).Hash();
            auto textureUniformInfo = m_glShader->GetUniformInfo(nameHash);
            assert(textureUniformInfo);

            textures.SetTexture(nameHash, nullptr);
        }
    }

    void Shader::CreatePredefinedCBuffer(cr<StringHandle> uniformBufferName)
    {
        if (GetGR()->NeedCreatePredefinedCbuffer(uniformBufferName))
        {
            GetGR()->CreatePredefinedCbuffer(uniformBufferName, msp<CBufferLayout>(m_glShader.get(), uniformBufferName.CStr()));
        }
    }

    void Shader::CombineSeparateTextures(spirv_cross::CompilerGLSL& compiler)
    {
        auto resources = compiler.get_shader_resources();
        std::unordered_map<uint32_t, std::string> previousName;
        for (auto& image : resources.separate_images)
        {
            previousName[image.id] = image.name;
        }
        
        compiler.build_combined_image_samplers();
        
        resources = compiler.get_shader_resources();
        for (auto& image : compiler.get_combined_image_samplers())
        {
            compiler.set_name(image.combined_id, previousName[image.image_id]);
        }
    }
}
