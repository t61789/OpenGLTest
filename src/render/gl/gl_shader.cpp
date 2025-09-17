#include "gl_shader.h"

#include "gl_state.h"

namespace op
{
    GlShader::GlShader(cr<std::string> preparedVert, cr<std::string> preparedFrag)
    {
        auto vCharSource = preparedVert.c_str();
        auto fCharSource = preparedFrag.c_str();
        
        GLuint vertexShader = GlState::GlGenShader(GL_VERTEX_SHADER);
        GlState::GlShaderSource(vertexShader, 1, &vCharSource, nullptr);
        GlState::GlCompileShader(vertexShader);
        CheckShaderCompilation(vertexShader, preparedVert);

        GLuint fragShader = GlState::GlGenShader(GL_FRAGMENT_SHADER);
        GlState::GlShaderSource(fragShader, 1, &fCharSource, nullptr);
        GlState::GlCompileShader(fragShader);
        CheckShaderCompilation(fragShader, preparedFrag);

        m_id = GlState::GlGenProgram();
        GlState::GlAttachShader(m_id, vertexShader);
        GlState::GlAttachShader(m_id, fragShader);
        GlState::GlLinkProgram(m_id);

        GlState::GlDeleteShader(vertexShader);
        GlState::GlDeleteShader(fragShader);

        m_uniforms = LoadUniforms(m_id);
    }

    GlShader::~GlShader()
    {
        GlState::GlDeleteProgram(m_id);
    }

    GlShader::UniformInfo* GlShader::GetUniformInfo(const string_hash name)
    {
        auto it = m_uniforms.find(name);
        if (it != m_uniforms.end())
        {
            return &it->second;
        }

        return nullptr;
    }

    void GlShader::Use()
    {
        GlState::Ins()->BindShader(shared_from_this());
    }

    void GlShader::SetInt(const string_hash name, const int32_t val)
    {
        assert(GlState::Ins()->GetShader() == shared_from_this());

        GlState::GlUniform1i(GetUniformInfo(name)->location, val);
    }

    void GlShader::SetFloat(const string_hash name, const float val)
    {
        assert(GlState::Ins()->GetShader() == shared_from_this());

        GlState::GlUniform1f(GetUniformInfo(name)->location, val);
    }

    void GlShader::SetFloat4(const string_hash name, const float* val)
    {
        assert(GlState::Ins()->GetShader() == shared_from_this());

        GlState::GlUniform4f(GetUniformInfo(name)->location, val[0], val[1], val[2], val[3]);
    }

    void GlShader::SetMatrix(const string_hash name, const float* val)
    {
        assert(GlState::Ins()->GetShader() == shared_from_this());

        GlState::GlUniformMatrix4fv(GetUniformInfo(name)->location, 1, GL_TRUE, val);
    }

    void GlShader::SetFloatArr(const string_hash name, const float* val, const uint32_t count)
    {
        assert(GlState::Ins()->GetShader() == shared_from_this());

        GlState::GlUniform1fv(GetUniformInfo(name)->location, count, val);
    }

    void GlShader::CheckShaderCompilation(const GLuint vertexShader, cr<std::string> source)
    {
        int success;
        GlState::GlGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char info[512];
            GlState::GlGetShaderInfoLog(vertexShader, 512, info);
            std::stringstream ss;
            ss << "ERROR>> Shader compilation failed:\n";
            ss << "\n";
            ss << info;
            ss << source.c_str();
            throw std::runtime_error(ss.str());
        }
    }
    
    umap<size_t, GlShader::UniformInfo> GlShader::LoadUniforms(const GLuint program)
    {
        umap<size_t, UniformInfo> result;
        
        GLint numUniforms = 0;
        GlState::GlGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

        for (int i = 0; i < numUniforms; ++i)
        {
            GLchar name[256];
            GLsizei length;
            GLint size;
            GLenum type;
            GlState::GlGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);

            UniformInfo uniformInfo;
            uniformInfo.name = name;
            uniformInfo.location = GlState::GetGlUniformLocation(program, name);
            uniformInfo.elemNum = size;
            uniformInfo.type = static_cast<int>(type);
            
            if (type == GL_FLOAT && size > 1)
            {
                uniformInfo.name = uniformInfo.name.Str().substr(0, uniformInfo.name.Str().length() - 3);
            }

            result[uniformInfo.name] = uniformInfo;
        }

        return result;
    }
}
