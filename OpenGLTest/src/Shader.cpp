#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "GameFramework.h"
#include "Utils.h"
#include "gtc/type_ptr.hpp"

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

Shader::~Shader()
{
    glDeleteProgram(glShaderId);
}

void Shader::use(const Mesh* mesh) const
{
    glUseProgram(glShaderId);

    for (size_t i = 0; i < VERTEX_ATTRIB_NUM; ++i)
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
}

bool Shader::hasParam(const std::string &name) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    return location != -1;
}

void Shader::setBool(const std::string& name, const bool value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    setBool(location, value);
}

void Shader::setBool(const int& location, const bool value)
{
    if(location == -1)
    {
        return;
    }
    glUniform1i(location, value);
}

void Shader::setInt(const std::string& name, const int value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    setInt(location, value);
}

void Shader::setInt(const int& location, const int value)
{
    if(location == -1)
    {
        return;
    }
    glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, const float value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    setFloat(location, value);
}

void Shader::setFloat(const int& location, const float value)
{
    if(location == -1)
    {
        return;
    }
    glUniform1f(location, value);
}

void Shader::setVector(const std::string& name, const glm::vec4& value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    setVector(location, value);
}

void Shader::setVector(const int& location, const glm::vec4& value)
{
    if(location == -1)
    {
        return;
    }
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::setMatrix(const std::string& name, const glm::mat4& value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    setMatrix(location, value);
}

void Shader::setMatrix(const int& location, const glm::mat4& value)
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
    if(ResourceMgr::IsResourceRegistered(checkPath))
    {
        return ResourceMgr::GetRegisteredResource(checkPath);
    }
    
    string vSource, fSource;
    ifstream vFile, fFile;

    vFile.exceptions(ifstream::failbit | ifstream::badbit);
    fFile.exceptions(ifstream::failbit | ifstream::badbit);

    try
    {
        vFile.open(Utils::GetRealAssetPath(vertexPath));
        fFile.open(Utils::GetRealAssetPath(fragPath));

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
    result->glShaderId = glShaderId;
    ResourceMgr::RegisterResource(checkPath, result->id);
    Utils::LogInfo("成功载入VertShader " + vertexPath);
    Utils::LogInfo("成功载入FragShader " + fragPath);
    return result->id;
}

void Shader::setTexture(const std::string& name, const int slot, const RESOURCE_ID value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    setTexture(location, slot, value);
}

void Shader::setTexture(const int& location, const int slot, RESOURCE_ID value)
{
    auto texturePtr = ResourceMgr::GetPtr<Texture>(value);
    if(location == -1 && texturePtr != nullptr)
    {
        return;
    }
    setInt(location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texturePtr->glTextureId);
}
