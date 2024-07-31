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

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, const bool value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    setBool(location, value);
}

void Shader::setBool(const int& name, const bool value)
{
    glUniform1i(name, value);
}

void Shader::setInt(const std::string& name, const int value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    setInt(location, value);
}

void Shader::setInt(const int& name, const int value)
{
    glUniform1i(name, value);
}

void Shader::setFloat(const std::string& name, const float value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    setFloat(location, value);
}

void Shader::setFloat(const int& name, const float value)
{
    glUniform1f(name, value);
}


