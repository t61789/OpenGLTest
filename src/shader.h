#pragma once

#include <string>
#include <spirv_glsl.hpp>

#include "shared_object.h"
#include "mesh.h"
#include "texture.h"
#include "render/cbuffer.h"

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
            bool hasInitGlVal = false;

            union
            {
                int intVal = 0;
                float floatVal;
            };

            static const UniformInfo& GetUnavailable()
            {
                static UniformInfo unavailable = { -1, -1, -1, "" };
                return unavailable;
            }
        };

        class UniformBlockMemberInfo
        {
        public:
            StringHandle name;
            size_t size;
            uint32_t offset;
            size_t blockNameId;
        };

        class UniformBlockInfo
        {
        public:
            StringHandle name;
            uint32_t binding;
            size_t size;
            std::vector<size_t> uniforms;
        };

        struct VertexLayoutInfo
        {
            uint32_t location;
        };

        struct TextureInfo
        {
            GLuint type;
            uint32_t location;
        };
        
        GLuint glShaderId;

        std::unordered_map<VertexAttr, VertexLayoutInfo> vertexLayout;
        std::unordered_map<size_t, UniformInfo> uniforms;
        std::unordered_map<size_t, UniformBlockInfo> uniformBlocks;
        std::unordered_map<size_t, UniformBlockMemberInfo> uniformMembers;

        std::unordered_map<size_t, TextureInfo> textures;
        std::unordered_map<size_t, CBufferLayout*> cbuffers;

        void Use(const Mesh* mesh) const;
        void Use0(const Mesh* mesh) const;
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

        template <typename T>
        void SetVal(size_t nameId, T value);

        static Shader* LoadFromFile(const std::string& glslPath);
        static Shader* LoadFromFile(const std::string& preparedVert, const std::string& preparedFrag, const std::string& glslPath = NOT_A_FILE);
        static Shader* LoadFromSpvBase64(const std::string& vert, const std::string& frag, const std::string& path = NOT_A_FILE);
        static Shader* LoadFromSpvBinary(std::vector<uint32_t> vert, std::vector<uint32_t> frag, const std::string& path = NOT_A_FILE);

    private:
        ~Shader() override;

        void LoadVertexLayout(const spirv_cross::CompilerGLSL& vertCompiler, const spirv_cross::ShaderResources& vertResources);
        void LoadCBuffer(const spirv_cross::CompilerGLSL& compiler, const spirv_cross::ShaderResources& resources);
        void LoadTextures(const spirv_cross::CompilerGLSL& compiler, const spirv_cross::ShaderResources& resources);
        
        template <typename T>
        void SetVal(size_t nameId, T value, T UniformInfo::* valueField, const std::function<void(int, T)>& glSetValue);

        static std::unordered_map<size_t, UniformInfo> LoadUniforms(GLuint program);
        static std::vector<std::string> LoadFileToLines(const std::string& realAssetPath);
        static void DivideGlsl(const std::vector<std::string>& lines, std::vector<std::string>& vertLines, std::vector<std::string>& fragLines);
        static void ReplaceIncludes(const std::string& curFilePath, std::vector<std::string>& lines);
        static void AddBuiltInMarcos(std::vector<std::string>& lines, const std::vector<std::string>& marcos);
        static std::vector<uint32_t> LoadSpvFileData(const std::string& absolutePath);
        static bool TryCreatePredefinedCBuffer(const spirv_cross::CompilerGLSL& compiler, const spirv_cross::Resource& uniformBuffer);
        static void CombineSeparateTextures(spirv_cross::CompilerGLSL& compiler);
        static void CheckShaderCompilation(GLuint vertexShader, const std::string &shaderPath, const std::string& source);
    };

    template <>
    inline void Shader::SetVal<int>(const size_t nameId, const int value)
    {
        static auto field = &UniformInfo::intVal;
        static std::function glSetValue = [](const int location, const int v) { glUniform1i(location, v); };
        SetVal(nameId, value, field, glSetValue);
    }

    template <>
    inline void Shader::SetVal<float>(const size_t nameId, const float value)
    {
        static auto field = &UniformInfo::floatVal;
        static std::function glSetValue = [](const int location, const float v) { glUniform1f(location, v); };
        SetVal(nameId, value, field, glSetValue);
    }

    template <typename T>
    void Shader::SetVal(const size_t nameId, T value, T UniformInfo::* valueField, const std::function<void(int, T)>& glSetValue)
    {
        auto it = uniforms.find(nameId);
        if (it == uniforms.end())
        {
            return;
        }

        auto& uniform = it->second;
        if (uniform.*valueField != value || !uniform.hasInitGlVal)
        {
            uniform.*valueField = value;
            glSetValue(uniform.location, value);
            uniform.hasInitGlVal = true;
        }
    }
}
