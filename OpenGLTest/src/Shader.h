#pragma once

#include <string>

#include "Mesh.h"
#include "Texture.h"
#include "glm.hpp"

class Shader : public SharedObject
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
    void SetTexture(const std::string& name, const int slot, const Texture* value) const;
    static void SetTexture(const int& location, const int slot, const Texture* value);
    void SetFloatArr(const std::string& name, uint32_t count, float* value) const;
    static void SetFloatArr(const int& location, int count, float *value);

    static Shader* LoadFromFile(const std::string &glslPath);

private:
    ~Shader() override;

    static std::vector<std::string> LoadFileToLines(const std::string& realAssetPath);
    static void DivideGlsl(const std::vector<std::string>& lines, std::vector<std::string>& vertLines, std::vector<std::string>& fragLines);
    static void ReplaceIncludes(const std::string& curFilePath, std::vector<std::string>& lines);
};
