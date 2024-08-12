#pragma once

#include <glad/glad.h>
#include "glm.hpp"
#include "gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>

#include "Texture.h"

class Shader
{
public:
    GLuint ID;

    Shader();
    Shader(const char* vertexPath, const char* fragPath);
    ~Shader();

    void use() const;
    bool hasParam(const std::string &name) const;
    void setBool(const std::string &name, bool value) const;
    static void setBool(const int &name, bool value);
    void setInt(const std::string &name, int value) const;
    static void setInt(const int &name, int value);
    void setFloat(const std::string &name, float value) const;
    static void setFloat(const int &name, float value);
    void setMatrix(const std::string& name, const glm::mat4& value) const;
    static void setMatrix(const int& name, const glm::mat4& value);
    void setTexture(const std::string& name, const int slot, const Texture* value) const;
    static void setTexture(const int& name, const int slot, const Texture* value);
};
