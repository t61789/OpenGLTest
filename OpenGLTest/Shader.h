#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragPath);
    ~Shader();

    void use() const;
    void setBool(const std::string &name, bool value) const;
    static void setBool(const int &name, bool value);
    void setInt(const std::string &name, int value) const;
    static void setInt(const int &name, int value);
    void setFloat(const std::string &name, float value) const;
    static void setFloat(const int &name, float value);
};