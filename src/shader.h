#pragma once

#include <string>

#include "shared_object.h"
#include "mesh.h"
#include "texture.h"

namespace op
{
    class Shader : public SharedObject
    {
    public:
        class UniformInfo
        {
        public:
            int location;
            int elemNum;
            int type;
            std::string name;

            bool operator==(const UniformInfo& other) const
            {
                return location == other.location && elemNum == other.elemNum && type == other.type && name == other.name;
            }

            static const UniformInfo& GetUnavailable()
            {
                static UniformInfo unavailable = { -1, -1, -1, "" };
                return unavailable;
            }
        };
        
        GLuint glShaderId;

        std::unordered_map<size_t, UniformInfo> uniforms;

        void Use(const Mesh* mesh) const;
        const UniformInfo& GetUniformInfo(size_t nameId) const;
        bool HasParam(const size_t& nameId) const;
        
        void SetBool(size_t nameId, bool value) const;
        static void SetBoolGl(int location, bool value);
        void SetInt(size_t nameId, int value) const;
        static void SetIntGl(int location, int value);
        void SetFloat(size_t nameId, float value) const;
        static void SetFloatGl(int location, float value);
        void SetVector(size_t nameId, const Vec4& value) const;
        static void SetVectorGl(int location, const Vec4& value);
        void SetMatrix(size_t nameId, const Matrix4x4& value) const;
        static void SetMatrixGl(int location, const Matrix4x4& value);
        void SetFloatArr(size_t nameId, int count, const float* value) const;
        static void SetFloatArrGl(int location, int count, const float* value);
        void SetTexture(size_t nameId, int slot, const Texture* value) const;
        static void SetTextureGl(int location, int slot, const Texture* value);

        static Shader* LoadFromFile(const std::string &glslPath);

    private:
        ~Shader() override;

        static std::unordered_map<size_t, UniformInfo> LoadUniforms(GLuint program);
        static std::vector<std::string> LoadFileToLines(const std::string& realAssetPath);
        static void DivideGlsl(const std::vector<std::string>& lines, std::vector<std::string>& vertLines, std::vector<std::string>& fragLines);
        static void ReplaceIncludes(const std::string& curFilePath, std::vector<std::string>& lines);
        static void AddBuiltInMarcos(std::vector<std::string>& lines, const std::vector<std::string>& marcos);
    };
}
