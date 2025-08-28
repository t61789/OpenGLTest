#include "shader.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_set>
#include <utility>

#include "game_framework.h"
#include "render_texture.h"
#include "string_handle.h"
#include "utils.h"
#include "const.h"
#include "game_resource.h"

namespace op
{
    using namespace std;

    Shader::~Shader()
    {
        glDeleteProgram(glShaderId);
        for (const auto& [nameId, cbufferLayout] : cbuffers)
        {
            DECREF(cbufferLayout)
        }
    }

    const Shader::UniformInfo& Shader::GetUniformInfo(const size_t nameId) const
    {
        auto it = uniforms.find(nameId);
        if (it != uniforms.end())
        {
            return it->second;
        }

        return UniformInfo::GetUnavailable();
    }

    bool Shader::HasParam(const size_t &nameId) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        return uniformInfo.location != -1;
    }

    void Shader::SetBool(const size_t nameId, const bool value) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        SetBoolGl(uniformInfo.location, value);
    }
    
    void Shader::SetBoolGl(const int location, const bool value)
    {
        if (location != -1)
        {
            glUniform1i(location, value);
        }
    }
    
    void Shader::SetInt(const size_t nameId, const int value) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        SetIntGl(uniformInfo.location, value);
    }
    
    void Shader::SetIntGl(const int location, const int value)
    {
        if (location != -1)
        {
            glUniform1i(location, value);
        }
    }
    
    void Shader::SetFloat(const size_t nameId, const float value) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        SetFloatGl(uniformInfo.location, value);
    }
    
    void Shader::SetFloatGl(const int location, const float value)
    {
        if (location != -1)
        {
            glUniform1f(location, value);
        }
    }

    void Shader::SetVector(const size_t nameId, const Vec4& value) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        SetVectorGl(uniformInfo.location, value);
    }
    
    void Shader::SetVectorGl(const int location, const Vec4& value)
    {
        if (location != -1)
        {
            glUniform4f(location, value.x, value.y, value.z, value.w);
        }
    }

    void Shader::SetMatrix(const size_t nameId, const Matrix4x4& value) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        SetMatrixGl(uniformInfo.location, value);
    }
    
    void Shader::SetMatrixGl(const int location, const Matrix4x4& value)
    {
        if (location != -1)
        {
            glUniformMatrix4fv(location, 1, GL_FALSE, value.Transpose().GetData());
        }
    }

    void Shader::SetFloatArr(const size_t nameId, const int count, const float* value) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        SetFloatArrGl(uniformInfo.location, count, value);
    }
    
    void Shader::SetFloatArrGl(const int location, const int count, const float* value)
    {
        if (location != -1)
        {
            glUniform1fv(location, count, value);
        }
    }

    void Shader::SetTexture(const size_t nameId, const int slot, const Texture* value) const
    {
        const auto& uniformInfo = GetUniformInfo(nameId);
        SetTextureGl(uniformInfo.location, slot, value);
    }
    
    void Shader::SetTextureGl(const int location, const int slot, const Texture* value)
    {
        if (location != -1 && value && value->isCreated)
        {
            SetIntGl(location, slot);
            glActiveTexture(GL_TEXTURE0 + slot);
            if(value->isCubeMap)
            {
                glBindTexture(GL_TEXTURE_CUBE_MAP, value->glTextureId);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, value->glTextureId);
            }
        }
    }

    Shader* Shader::LoadFromFile(const string& path)
    {
        {
            SharedObject* result;
            if(TryGetResource(path, result))
            {
                return dynamic_cast<Shader*>(result);
            }
        }

        THROW_ERRORF("Shader未在pack中找到：%s", path.c_str())
    }
    
    Shader* Shader::LoadFromFile(const std::string& preparedVert, const std::string& preparedFrag, const std::string& glslPath)
    {
        auto vCharSource = preparedVert.c_str();
        auto fCharSource = preparedFrag.c_str();
        
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vCharSource, nullptr);
        glCompileShader(vertexShader);
        CheckShaderCompilation(vertexShader, glslPath, preparedVert);

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fCharSource, nullptr);
        glCompileShader(fragShader);
        CheckShaderCompilation(fragShader, glslPath, preparedFrag);

        auto glShaderId = glCreateProgram();
        glAttachShader(glShaderId, vertexShader);
        glAttachShader(glShaderId, fragShader);
        glLinkProgram(glShaderId);

        glDeleteShader(vertexShader);
        glDeleteShader(fragShader);

        GL_CHECK_ERROR(创建Shader)

        auto result = new Shader();
        result->glShaderId = glShaderId;
        result->uniforms = LoadUniforms(glShaderId);
        RegisterResource(glslPath, result);
        log_info("成功载入Shader " + glslPath);
        return result;
    }

    Shader* Shader::LoadFromSpvBase64(const std::string& vert, const std::string& frag, const std::string& path)
    {
        auto vertStr = Utils::Binary8To32(Utils::Base64ToBinary(vert));
        auto fragStr = Utils::Binary8To32(Utils::Base64ToBinary(frag));

        return LoadFromSpvBinary(std::move(vertStr), std::move(fragStr), path);
    }

    Shader* Shader::LoadFromSpvBinary(std::vector<uint32_t> vert, std::vector<uint32_t> frag, const std::string& path)
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
        
        auto result = LoadFromFile(vSource, fSource, path);

        // Load VertexLayout
        result->LoadVertexLayout(vertCompilerGlsl, vertShaderResources);

        // Load CBuffer
        result->LoadCBuffer(vertCompilerGlsl, vertShaderResources);
        result->LoadCBuffer(fragCompilerGlsl, fragShaderResources);

        // Load Textures
        result->LoadTextures(vertCompilerGlsl, vertShaderResources);
        result->LoadTextures(fragCompilerGlsl, fragShaderResources);

        if (path.find("indirect_test") != std::string::npos)
        {
            log_info(vSource);
            log_info(fSource);
        }

        return result;
    }

    unordered_map<size_t, Shader::UniformInfo> Shader::LoadUniforms(const GLuint program)
    {
        unordered_map<size_t, UniformInfo> result;
        
        GLint numUniforms = 0;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

        for (int i = 0; i < numUniforms; ++i)
        {
            GLchar name[256];
            GLsizei length;
            GLint size;
            GLenum type;
            glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);

            UniformInfo uniformInfo;
            uniformInfo.name = string(name);
            uniformInfo.location = glGetUniformLocation(program, name);
            uniformInfo.elemNum = size;
            uniformInfo.type = static_cast<int>(type);
            
            if (type == GL_FLOAT && size > 1)
            {
                uniformInfo.name = uniformInfo.name.substr(0, uniformInfo.name.length() - 3);
            }

            result[StringHandle(uniformInfo.name).Hash()] = uniformInfo;
        }

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

    void Shader::LoadVertexLayout(const spirv_cross::CompilerGLSL& vertCompiler, const spirv_cross::ShaderResources& vertResources)
    {
        for (const auto& stageInput : vertResources.stage_inputs)
        {
            auto found = false;
            for (const auto& [attr, attrName] : VERTEX_ATTR_NAME)
            {
                if (!ends_with(stageInput.name, attrName))
                {
                    continue;
                }

                auto location = vertCompiler.get_decoration(stageInput.id, spv::DecorationLocation);

                vertexLayout[attr] = { location };
                found = true;
                break;
            }

            if (!found)
            {
                throw runtime_error(format_string("无法找到顶点属性 %s", stageInput.name.c_str()));
            }
        }
    }

    void Shader::LoadCBuffer(
        const spirv_cross::CompilerGLSL& compiler,
        const spirv_cross::ShaderResources& resources)
    {
        for (const auto& uniformBuffer : resources.uniform_buffers)
        {
            // 是内置cbuffer的话就加载到GameResource中
            if (TryCreatePredefinedCBuffer(compiler, uniformBuffer))
            {
                continue;
            }

            // 没加载过就加载，由当前Shader持有这个cbuffer
            auto uniformBufferNameId = StringHandle(uniformBuffer.name).Hash();
            if (cbuffers.find(uniformBufferNameId) != cbuffers.end())
            {
                continue;
            }

            auto cbuffer = new CBufferLayout(compiler, uniformBuffer);
            cbuffers[uniformBufferNameId] = cbuffer;
            INCREF(cbuffer)
        }
    }

    void Shader::LoadTextures(
        const spirv_cross::CompilerGLSL& compiler,
        const spirv_cross::ShaderResources& resources)
    {
        for (const auto& image : resources.sampled_images)
        {
            const auto& type = compiler.get_type(image.type_id);

            if (type.basetype != spirv_cross::SPIRType::SampledImage)
            {
                continue;
            }

            GLuint textureType;
            if (type.image.dim == spv::Dim2D)
            {
                textureType = GL_TEXTURE_2D;
            }
            else if (type.image.dim == spv::DimCube)
            {
                textureType = GL_TEXTURE_CUBE_MAP;
            }
            else
            {
                THROW_ERROR("不支持的纹理类型")
            }

            uint32_t location = glGetUniformLocation(glShaderId, image.name.c_str());
            textures[StringHandle(image.name).Hash()] = { textureType, location };
        }
    }

    bool Shader::TryCreatePredefinedCBuffer(
        const spirv_cross::CompilerGLSL& compiler,
        const spirv_cross::Resource& uniformBuffer)
    {
        auto uniformBufferNameId = StringHandle(uniformBuffer.name).Hash();
        if (uniformBufferNameId == StringHandle("ObjectIndexCBuffer")) // TODO
        {
            return true;
        }
        
        if (auto predefinedMaterial = GetGR()->GetPredefinedMaterial(uniformBufferNameId))
        {
            if (!predefinedMaterial->HasCBuffer())
            {
                predefinedMaterial->CreateCBuffer(new CBufferLayout(compiler, uniformBuffer));
            }
            
            return true;
        }

        return false;
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

    void Shader::CheckShaderCompilation(const GLuint vertexShader, const string &shaderPath, const string& source)
    {
        int success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char info[512];
            glGetShaderInfoLog(vertexShader, 512, nullptr, info);
            stringstream ss;
            ss << "ERROR>> Shader compilation failed:\n";
            ss << shaderPath;
            ss << "\n";
            ss << info;
            ss << source.c_str();
            throw runtime_error(ss.str());
        }
    }
}
