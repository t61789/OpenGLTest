#pragma once

#include <glad/glad.h>
#include "glm.hpp"
#include "gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>

#include "Mesh.h"
#include "Texture.h"

class Shader
{
public:
    RESOURCE_ID m_id;
    GLuint m_glShaderId;

    void Use(Mesh* mesh) const;
    bool HasParam(const std::string &name) const;
    void SetBool(const std::string &name, bool value) const;
    static void SetBool(const int &location, bool value);
    void SetInt(const std::string &name, int value) const;
    static void SetInt(const int &location, int value);
    void SetFloat(const std::string &name, float value) const;
    static void SetFloat(const int &location, float value);
    void SetVector(const std::string &name, glm::vec4 value) const;
    static void SetVector(const int &location, glm::vec4 value);
    void SetMatrix(const std::string& name, const glm::mat4& value) const;
    static void SetMatrix(const int& location, const glm::mat4& value);
    void SetTexture(const std::string& name, const int slot, const RESOURCE_ID value) const;
    static void SetTexture(const int& location, const int slot, const RESOURCE_ID value);

    static RESOURCE_ID LoadFromFile(const std::string &vertexPath, const std::string &fragPath);

private:
    Shader();
    ~Shader();
};
