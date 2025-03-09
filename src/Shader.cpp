#include "Shader.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_set>

#include "GameFramework.h"
#include "RenderTexture.h"
#include "Utils.h"
#include "glm/gtc/type_ptr.hpp"

using namespace std;

string getShaderStr(const vector<string>& lines)
{
    stringstream ss;
    for (int i = 0; i < lines.size(); ++i)
    {
        ss << to_string(i + 1) << ":  ";
        ss << lines[i];
        ss << "\n";
    }
    return ss.str();
}

void checkShaderCompilation(const GLuint vertexShader, const string &shaderPath, const vector<string>& lines)
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
        ss << getShaderStr(lines);
        throw runtime_error(ss.str());
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

bool Shader::HasParam(const string &name) const
{
    int location = glGetUniformLocation(glShaderId, name.c_str());
    return location != -1;
}

void Shader::SetBool(const string& name, const bool value) const
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

void Shader::SetInt(const string& name, const int value) const
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

void Shader::SetFloat(const string& name, const float value) const
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

void Shader::SetVector(const string& name, const glm::vec4& value) const
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

void Shader::SetMatrix(const string& name, const glm::mat4& value) const
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

void Shader::SetTexture(const string& name, const int slot, const Texture* value) const
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

void Shader::SetFloatArr(const string& name, uint32_t count, float* value) const
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
        auto includeLines = LoadFileToLines(Utils::GetRealAssetPath(includePath));
        lines.insert(lines.begin() + i, includeLines.begin(), includeLines.end());
        i--;
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
    vector<string> glslLines = LoadFileToLines(Utils::GetRealAssetPath(glslPath));
    vector<string> vertLines;
    vector<string> fragLines;

    DivideGlsl(glslLines, vertLines, fragLines);
    
    try
    {
        ReplaceIncludes(glslPath, vertLines);
        ReplaceIncludes(glslPath, fragLines);

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
    result->uniforms = LoadUniforms(glShaderId);
    RegisterResource(glslPath, result);
    Utils::LogInfo("成功载入Shader " + glslPath);
    return result;
}

vector<Shader::UniformInfo> Shader::LoadUniforms(const GLuint program)
{
    vector<UniformInfo> result;
    
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
        uniformInfo.index = i;
        uniformInfo.name = string(name);
        uniformInfo.elemNum = size;
        uniformInfo.type = type;

        if (find(result.begin(), result.end(), uniformInfo) == result.end())
        {
            result.push_back(uniformInfo);
        }
        else
        {
            throw runtime_error("重复的Uniform");
        }
    }

    return result;
}
