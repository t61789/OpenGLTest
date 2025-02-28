﻿#include "Shader.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <unordered_set>

#include "GameFramework.h"
#include "RenderTexture.h"
#include "Utils.h"
#include "gtc/type_ptr.hpp"

std::string getShaderStr(const std::vector<std::string>& lines)
{
    std::stringstream ss;
    for (int i = 0; i < lines.size(); ++i)
    {
        ss << std::to_string(i + 1) << ":  ";
        ss << lines[i];
        ss << "\n";
    }
    return ss.str();
}

void checkShaderCompilation(const GLuint vertexShader, const std::string &shaderPath, const std::vector<std::string>& lines)
{
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char info[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, info);
        std::stringstream ss;
        ss << "ERROR>> Shader compilation failed:\n";
        ss << shaderPath;
        ss << "\n";
        ss << info;
        ss << getShaderStr(lines);
        throw std::runtime_error(ss.str());
    }
}
// TODO 判断参数的类型，类型不对不设置或者报错
Shader::~Shader()
{
    glDeleteProgram(glShaderId);
}

void Shader::Use(const Mesh* mesh) const
{
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
}

bool Shader::HasParam(const std::string &name) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    return location != -1;
}

void Shader::SetBool(const std::string& name, const bool value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
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
    int location = glGetUniformLocation(glShaderId, name.c_str());
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
    int location = glGetUniformLocation(glShaderId, name.c_str());
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

void Shader::SetVector(const std::string& name, const glm::vec4& value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    SetVector(location, value);
}

void Shader::SetVector(const int& location, const glm::vec4& value)
{
    if(location == -1)
    {
        return;
    }
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::SetMatrix(const std::string& name, const glm::mat4& value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
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

void Shader::SetTexture(const std::string& name, const int slot, const Texture* value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    SetTexture(location, slot, value);
}

void Shader::SetTexture(const int& location, const int slot, const Texture* value)
{
    if(location == -1 || value == nullptr || !value->isCreated)
    {
        return;
    }
    
    SetInt(location, slot);
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

void Shader::SetFloatArr(const std::string& name, uint32_t count, float* value) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    SetFloatArr(location, count, value);
}

void Shader::SetFloatArr(const int& location, int count, float* value)
{
    if(location == -1)
    {
        return;
    }
    glUniform1fv(location, count, value);
}

std::vector<std::string> Shader::LoadFileToLines(const std::string& realAssetPath)
{
    std::ifstream fs;
    fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::stringstream ss;
    try
    {
        fs.open(realAssetPath);
        ss << fs.rdbuf();
    }
    catch(std::exception&)
    {
        fs.close();
        Utils::LogError("读取文件失败：" + realAssetPath);
        throw;
    }
    fs.close();
    
    std::vector<std::string> lines;
    std::string line;
    while(std::getline(ss, line, '\n'))
    {
        lines.push_back(line);
    }

    return lines;
}

void Shader::DivideGlsl(const std::vector<std::string>& lines, std::vector<std::string>& vertLines, std::vector<std::string>& fragLines)
{
    int divideIndex[2];
    int curIndex = 0;
    for (int i = 0; i < lines.size(); ++i)
    {
        if(lines[i].find("#version") != std::string::npos)
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
        throw std::runtime_error("GLSL文件需求两个#version，但只找到" + std::to_string(curIndex) + "个");
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

void Shader::ReplaceIncludes(const std::string& curFilePath, std::vector<std::string>& lines)
{
    std::vector<size_t> loadStack;
    std::vector<std::string> currentFilePath;
    std::unordered_set<std::string> hasInclude;
    hasInclude.insert(curFilePath);
    loadStack.push_back(lines.size());
    currentFilePath.push_back(curFilePath);
    for (int i = 0; i < lines.size(); ++i)
    {
        loadStack[loadStack.size() - 1] -= 1;
        if(loadStack[loadStack.size() - 1] == 0)
        {
            loadStack.pop_back();
            currentFilePath.pop_back();
        }
        
        auto line = lines[i];
        auto includeCmdIndex = line.find("#include");
        if(includeCmdIndex == std::string::npos)
        {
            continue;
        }

        std::regex pattern("^\\s*#include\\s+\"([^\"]+)\"\\s*$");
        std::smatch matches;
        if(!std::regex_match(line, matches, pattern))
        {
            continue;
        }

        auto includePath = matches[1].str();
        lines.erase(lines.begin() + i);
        if(hasInclude.find(includePath) != hasInclude.end())
        {
            continue;
        }
        hasInclude.insert(includePath);
        currentFilePath.push_back(includePath);
        auto includeLines = LoadFileToLines(Utils::GetRealAssetPath(includePath, currentFilePath[currentFilePath.size() - 2]));
        loadStack.push_back(includeLines.size());
        
        lines.insert(lines.begin() + i, includeLines.begin(), includeLines.end());
        i--;
    }
}

Shader* Shader::LoadFromFile(const std::string& glslPath)
{
    {
        SharedObject* result;
        if(TryGetResource(glslPath, result))
        {
            return dynamic_cast<Shader*>(result);
        }
    }

    std::string vSource, fSource;
    std::vector<std::string> glslLines = LoadFileToLines(Utils::GetRealAssetPath(glslPath));
    std::vector<std::string> vertLines;
    std::vector<std::string> fragLines;

    DivideGlsl(glslLines, vertLines, fragLines);
    
    try
    {
        ReplaceIncludes(glslPath, vertLines);
        ReplaceIncludes(glslPath, fragLines);

        vSource = Utils::JoinStrings(vertLines, "\n");
        fSource = Utils::JoinStrings(fragLines, "\n");
    }
    catch (std::exception &e)
    {
        std::stringstream ss;
        ss << Utils::FormatLog(Error, glslPath) << "\n";
        ss << Utils::FormatLog(Error, "访问shader文件失败") << e.what();
        throw std::runtime_error(ss.str());
    }

    auto vCharSource = vSource.c_str();
    auto fCharSource = fSource.c_str();
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vCharSource, nullptr);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader, glslPath, vertLines);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fCharSource, nullptr);
    glCompileShader(fragShader);
    checkShaderCompilation(fragShader, glslPath, fragLines);

    auto glShaderId = glCreateProgram();
    glAttachShader(glShaderId, vertexShader);
    glAttachShader(glShaderId, fragShader);
    glLinkProgram(glShaderId);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    auto result = new Shader();
    result->glShaderId = glShaderId;
    RegisterResource(glslPath, result);
    Utils::LogInfo("成功载入Shader " + glslPath);
    return result;
}
