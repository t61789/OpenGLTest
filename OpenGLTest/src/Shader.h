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

    void Use() const;
    bool HasParam(const std::string &name) const;
    void SetBool(const std::string &name, bool value) const;
    static void SetBool(const int &name, bool value);
    void SetInt(const std::string &name, int value) const;
    static void SetInt(const int &name, int value);
    void SetFloat(const std::string &name, float value) const;
    static void SetFloat(const int &name, float value);
    void SetMatrix(const std::string& name, const glm::mat4& value) const;
    static void SetMatrix(const int& name, const glm::mat4& value);
    void SetTexture(const std::string& name, const int slot, const Texture* value) const;
    static void SetTexture(const int& name, const int slot, const Texture* value);
};
