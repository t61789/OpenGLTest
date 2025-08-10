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
            DECREF(cbufferLayout);
        }
    }

    void Shader::Use(const Mesh* mesh) const
    {
        if (!vertexLayout.empty())
        {
            Use0(mesh);
            return;
        }
        
        glUseProgram(glShaderId);

        for (int i = 0; i < VERTEX_ATTRIB_NUM; ++i)
        {
            int curLayout = glGetAttribLocation(glShaderId, VERTEX_ATTRIB_NAMES[i]);
            bool existInShader = curLayout != -1;
            bool existInMesh = mesh->vertexAttribEnabled[i];
            if(!existInShader || !existInMesh)
            {
                continue;
            }

            glVertexAttribPointer(
                curLayout,
                VERTEX_ATTRIB_FLOAT_COUNT[i],
                GL_FLOAT,
                GL_FALSE,
                sizeof(float) * mesh->vertexDataFloatNum,
                (void*)(mesh->vertexAttribOffset[i] * sizeof(float)));
            glEnableVertexAttribArray(curLayout);
        }

        Utils::CheckGlError("启用Shader");
    }
    
    void Shader::Use0(const Mesh* mesh) const
    {
        glUseProgram(glShaderId);

        for (const auto& [attr, shaderVertexLayout] : vertexLayout)
        {
            auto it = mesh->vertexAttribInfo.find(attr);
            if (it == mesh->vertexAttribInfo.end())
            {
                glDisableVertexAttribArray(shaderVertexLayout.location);
                continue;
            }

            const auto& meshVertexLayout = it->second;

            glVertexAttribPointer(
                shaderVertexLayout.location,
                static_cast<GLint>(VERTEX_ATTR_STRIDE[attr]),
                GL_FLOAT,
                GL_FALSE,
                static_cast<GLsizei>(mesh->vertexDataStride),
                reinterpret_cast<const void*>(meshVertexLayout.offset));
            glEnableVertexAttribArray(shaderVertexLayout.location);
        }

        Utils::CheckGlError("启用Shader");
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

    vector<string> Shader::LoadFileToLines(const string& realAssetPath)
    {
        ifstream fs;
        fs.exceptions(ifstream::failbit | ifstream::badbit);
        stringstream ss;
        try
        {
            fs.open(realAssetPath);
            ss << fs.rdbuf();
        }
        catch(exception&)
        {
            Utils::LogError("读取文件失败：" + realAssetPath);
            throw;
        }
        fs.close();
        
        vector<string> lines;
        string line;
        while(getline(ss, line, '\n'))
        {
            lines.push_back(line);
        }

        return lines;
    }

    void Shader::DivideGlsl(const vector<string>& lines, vector<string>& vertLines, vector<string>& fragLines)
    {
        int divideIndex[2];
        int curIndex = 0;
        for (int i = 0; i < lines.size(); ++i)
        {
            if(lines[i].find("#version") != string::npos)
            {
                divideIndex[curIndex] = i;
                curIndex ++;
                if(curIndex == 2)
                {
                    break;
                }
            }
        }

        if(curIndex != 2)
        {
            throw runtime_error("GLSL文件需求两个#version，但只找到" + to_string(curIndex) + "个");
        }

        for(int i = divideIndex[0]; i < divideIndex[1]; ++i)
        {
            vertLines.push_back(lines[i]);
        }
        
        for(int i = divideIndex[1]; i < lines.size(); ++i)
        {
            fragLines.push_back(lines[i]);
        }
    }

    void Shader::ReplaceIncludes(const string& curFilePath, vector<string>& lines)
    {
        unordered_set<string> hasInclude;
        hasInclude.insert(curFilePath);
        for (int i = 0; i < lines.size(); ++i)
        {
            // 如果当前行不是include就继续找
            auto line = lines[i];
            regex pattern("^\\s*#include\\s+\"([^\"]+)\"\\s*$");
            smatch matches;
            if(!regex_match(line, matches, pattern))
            {
                continue;
            }

            // 当前行是include
            auto includePath = matches[1].str();
            lines.erase(lines.begin() + i);
            if(hasInclude.find(includePath) != hasInclude.end())
            {
                // 这个文件已经include过了
                i--;
                continue;
            }
            hasInclude.insert(includePath);
            auto includeLines = LoadFileToLines(Utils::GetAbsolutePath(includePath));
            lines.insert(lines.begin() + i, includeLines.begin(), includeLines.end());
            i--;
        }
    }

    void Shader::AddBuiltInMarcos(std::vector<std::string>& lines, const std::vector<std::string>& marcos)
    {
        for (const auto& marco : marcos)
        {
            std::stringstream ss;
            ss << "#define " << marco;
            lines.insert(lines.begin() + 1, ss.str());
        }
    }

    Shader* Shader::LoadFromFile(const string& glslPath)
    {
        {
            SharedObject* result;
            if(TryGetResource(glslPath, result))
            {
                return dynamic_cast<Shader*>(result);
            }
        }

        string vSource, fSource;
        vector<string> glslLines = LoadFileToLines(Utils::GetAbsolutePath(glslPath));
        vector<string> vertLines;
        vector<string> fragLines;

        DivideGlsl(glslLines, vertLines, fragLines);
        
        try
        {
            ReplaceIncludes(glslPath, vertLines);
            AddBuiltInMarcos(vertLines, { "VERT_SHADER" });
            
            ReplaceIncludes(glslPath, fragLines);
            AddBuiltInMarcos(fragLines, { "FRAG_SHADER" });

            vSource = Utils::JoinStrings(vertLines, "\n");
            fSource = Utils::JoinStrings(fragLines, "\n");
        }
        catch (exception &e)
        {
            stringstream ss;
            ss << Utils::FormatLog(Error, glslPath) << "\n";
            ss << Utils::FormatLog(Error, "访问shader文件失败") << e.what();
            throw runtime_error(ss.str());
        }

        return LoadFromFile(vSource, fSource, glslPath);
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

        Utils::CheckGlError("创建Shader");

        auto result = new Shader();
        result->glShaderId = glShaderId;
        result->uniforms = LoadUniforms(glShaderId);
        RegisterResource(glslPath, result);
        Utils::LogInfo("成功载入Shader " + glslPath);
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
        options.version = 310;
        options.es = true;
        
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

        Utils::Log(Info, "%s ", vSource.c_str());
        Utils::Log(Info, "%s ", fSource.c_str());

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
            throw std::runtime_error("打开 SPIR-V 文件失败");
        }
        
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (size % sizeof(uint32_t) != 0)
        {
            throw std::runtime_error("SPIR-V file 大小不正确");
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
                if (!Utils::EndsWith(stageInput.name, attrName))
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
                throw runtime_error( Utils::FormatString("无法找到顶点属性 %s", stageInput.name.c_str()));
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
            INCREF(cbuffer);
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
            else if (type.image.dim != spv::DimCube)
            {
                textureType = GL_TEXTURE_CUBE_MAP;
            }
            else
            {
                throw std::runtime_error("不支持的纹理类型");
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
        if (auto predefinedMaterial = GameResource::GetInstance()->GetPredefinedMaterial(uniformBufferNameId))
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
