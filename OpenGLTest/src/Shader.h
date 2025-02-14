#pragma once

#include <string>

#include "Mesh.h"
#include "Texture.h"
#include "glm.hpp"

class Shader : public ResourceBase
{
public:
    GLuint glShaderId;

    void Use(const Mesh* mesh) const;
    bool HasParam(const std::string &name) const;
    void SetBool(const std::string &name, bool value) const;
    static void SetBool(const int &location, bool value);
    void SetInt(const std::string &name, int value) const;
    static void SetInt(const int &location, int value);
    void SetFloat(const std::string &name, float value) const;
    static void SetFloat(const int &location, float value);
    void SetVector(const std::string &name, const glm::vec4& value) const;
    static void SetVector(const int &location, const glm::vec4& value);
    void SetMatrix(const std::string& name, const glm::mat4& value) const;
    static void SetMatrix(const int& location, const glm::mat4& value);
    void SetTexture(const std::string& name, const int slot, const RESOURCE_ID value) const;
    static void SetTexture(const int& location, const int slot, const RESOURCE_ID value);
    void SetFloatArr(const std::string& name, int count, float *value) const;
    static void SetFloatArr(const int& location, int count, float *value);

    static RESOURCE_ID LoadFromFile(const std::string &glslPath);

private:
    ~Shader() override;
};
