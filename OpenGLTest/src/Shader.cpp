#include "Shader.h"

using namespace std;

void checkShaderCompilation(GLuint vertexShader, const char* shaderPath)
{
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char info[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, info);
        throw runtime_error(string("ERROR>> Shader compilation failed:\n") + string(shaderPath) + "\n" + info);
    }
}

Shader::Shader() = default;

Shader::Shader(const char* vertexPath, const char* fragPath)
{
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

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragShader);
    glLinkProgram(ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

void Shader::Use() const
{
    glUseProgram(ID);
}

bool Shader::HasParam(const std::string &name) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    return location != -1;
}

void Shader::SetBool(const std::string& name, const bool value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
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
    int location = glGetUniformLocation(ID, name.c_str());
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
    int location = glGetUniformLocation(ID, name.c_str());
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
    int location = glGetUniformLocation(ID, name.c_str());
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
    int location = glGetUniformLocation(ID, name.c_str());
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
    int location = glGetUniformLocation(ID, name.c_str());
    SetTexture(location, slot, value);
}

void Shader::SetTexture(const int& location, const int slot, const Texture* value)
{
    if(location == -1)
    {
        return;
    }
    SetInt(location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, value->textureId);
}
