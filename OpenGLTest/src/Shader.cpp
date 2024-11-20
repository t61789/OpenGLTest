#include "Shader.h"

#include <iostream>

#include "GameFramework.h"

using namespace std;

void checkShaderCompilation(const GLuint vertexShader, const std::string &shaderPath)
{
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char info[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, info);
        throw runtime_error(string("ERROR>> Shader compilation failed:\n") + shaderPath + "\n" + info);
    }
}

Shader::Shader()
{
    m_id = ResourceMgr::AddPtr(this);
}

Shader::~Shader()
{
    glDeleteProgram(m_glShaderId);

    ResourceMgr::RemovePtr(m_id);
}

void Shader::Use(Mesh* mesh) const
{
    glUseProgram(m_glShaderId);

    for (size_t i = 0; i < VERTEX_ATTRIB_NUM; ++i)
    {
        int curLayout = glGetAttribLocation(m_glShaderId, VERTEX_ATTRIB_NAMES[i]);
        bool existInShader = curLayout != -1;
        bool existInMesh = mesh->m_vertexAttribEnabled[i];
        if(!existInShader || !existInMesh)
        {
            continue;
        }

        glVertexAttribPointer(
            curLayout,
            VERTEX_ATTRIB_FLOAT_COUNT[i],
            GL_FLOAT,
            GL_FALSE,
            sizeof(float) * mesh->vertexDataStride,
            (void*)(mesh->m_vertexAttribOffset[i] * sizeof(float)));
        glEnableVertexAttribArray(curLayout);
    }
}

bool Shader::HasParam(const std::string &name) const
{
    int location = glGetUniformLocation(m_glShaderId, name.c_str());
    return location != -1;
}

void Shader::SetBool(const std::string& name, const bool value) const
{
    int location = glGetUniformLocation(m_glShaderId, name.c_str());
    SetBool(location, value);
}

void Shader::SetBool(const int& location, const bool value)
{
    if(location == -1)
    {
        return;
    }
    glUniform1i(location, value);
}

void Shader::SetInt(const std::string& name, const int value) const
{
    int location = glGetUniformLocation(m_glShaderId, name.c_str());
    SetInt(location, value);
}

void Shader::SetInt(const int& location, const int value)
{
    if(location == -1)
    {
        return;
    }
    glUniform1i(location, value);
}

void Shader::SetFloat(const std::string& name, const float value) const
{
    int location = glGetUniformLocation(m_glShaderId, name.c_str());
    SetFloat(location, value);
}

void Shader::SetFloat(const int& location, const float value)
{
    if(location == -1)
    {
        return;
    }
    glUniform1f(location, value);
}

void Shader::SetVector(const std::string& name, glm::vec4 value) const
{
    int location = glGetUniformLocation(m_glShaderId, name.c_str());
    SetVector(location, value);
}

void Shader::SetVector(const int& location, glm::vec4 value)
{
    if(location == -1)
    {
        return;
    }
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::SetMatrix(const std::string& name, const glm::mat4& value) const
{
    int location = glGetUniformLocation(m_glShaderId, name.c_str());
    SetMatrix(location, value);
}

void Shader::SetMatrix(const int& location, const glm::mat4& value)
{
    if(location == -1)
    {
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

RESOURCE_ID Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragPath)
{
    std::string checkPath = vertexPath + fragPath;
    if(ResourceMgr::HasResourceRegistered(checkPath))
    {
        return ResourceMgr::GetResourceId(checkPath);
    }
    
    string vSource, fSource;
    ifstream vFile, fFile;

    vFile.exceptions(ifstream::failbit | ifstream::badbit);
    fFile.exceptions(ifstream::failbit | ifstream::badbit);

    try
    {
        vFile.open(vertexPath);
        fFile.open(fragPath);

        stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();

        vFile.close();
        fFile.close();

        vSource = vStream.str();
        fSource = fStream.str();
    }
    catch (exception &e)
    {
        vFile.close();
        fFile.close();
        
        throw runtime_error(string("ERROR>> Failed to access shader files: ") + + e.what());
    }

    auto vCharSource = vSource.c_str();
    auto fCharSource = fSource.c_str();
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vCharSource, nullptr);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader, vertexPath);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fCharSource, nullptr);
    glCompileShader(fragShader);
    checkShaderCompilation(fragShader, fragPath);

    auto glShaderId = glCreateProgram();
    glAttachShader(glShaderId, vertexShader);
    glAttachShader(glShaderId, fragShader);
    glLinkProgram(glShaderId);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    auto result = new Shader();
    result->m_glShaderId = glShaderId;
    ResourceMgr::RegisterResource(checkPath, result->m_id);
    return result->m_id;
}

void Shader::SetTexture(const std::string& name, const int slot, const RESOURCE_ID value) const
{
    int location = glGetUniformLocation(m_glShaderId, name.c_str());
    SetTexture(location, slot, value);
}

void Shader::SetTexture(const int& location, const int slot, RESOURCE_ID value)
{
    auto texturePtr = ResourceMgr::GetPtr<Texture>(value);
    if(location == -1 && texturePtr != nullptr)
    {
        return;
    }
    SetInt(location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texturePtr->m_glTextureId);
}
