#pragma once

#include <string>

#include "Mesh.h"
#include "Texture.h"
#include "glm.hpp"

class Shader : public ResourceBase
{
public:
    GLuint glShaderId;

    void use(const Mesh* mesh) const;
    bool hasParam(const std::string &name) const;
    void setBool(const std::string &name, bool value) const;
    static void setBool(const int &location, bool value);
    void setInt(const std::string &name, int value) const;
    static void setInt(const int &location, int value);
    void setFloat(const std::string &name, float value) const;
    static void setFloat(const int &location, float value);
    void setVector(const std::string &name, const glm::vec4& value) const;
    static void setVector(const int &location, const glm::vec4& value);
    void setMatrix(const std::string& name, const glm::mat4& value) const;
    static void setMatrix(const int& location, const glm::mat4& value);
    void setTexture(const std::string& name, const int slot, const RESOURCE_ID value) const;
    static void setTexture(const int& location, const int slot, const RESOURCE_ID value);

    static RESOURCE_ID LoadFromFile(const std::string &vertexPath, const std::string &fragPath);

private:
    ~Shader() override;
};
