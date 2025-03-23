#pragma once
#include <string>
#include <unordered_map>

#include "blend_mode.h"
#include "cull_mode.h"
#include "shared_object.h"
#include "glm/glm.hpp"

namespace op
{
    class Shader;
    class Texture;
    class Mesh;

    class Material : public SharedObject
    {
    public:
        std::string name = "Unnamed Material";

        Shader* shader = nullptr;

        CullMode cullMode = CullMode::Back;
        BlendMode blendMode = BlendMode::None;

        std::unordered_map<std::string, int> intValues;
        std::unordered_map<std::string, bool> boolValues;
        std::unordered_map<std::string, float> floatValues;
        std::unordered_map<std::string, glm::mat4> mat4Values;
        std::unordered_map<std::string, Texture*> textureValues;
        std::unordered_map<std::string, glm::vec4> vec4Values;
        std::unordered_map<std::string, std::vector<float>*> floatArrValues;
        
        Material() = default;
        Material(const std::string& name);
        ~Material() override;

        void SetIntValue(const std::string& paramName, int value);
        void SetBoolValue(const std::string& paramName, bool value);
        void SetFloatValue(const std::string& paramName, float value);
        void SetMat4Value(const std::string& paramName, const glm::mat4& value);
        void SetTextureValue(const std::string& paramName, Texture* value);
        void SetVector4Value(const std::string& paramName, const glm::vec4& value);
        void SetFloatArrValue(const std::string& paramName, const float *value, int count);

        void FillParams(const Shader* targetShader) const;
        void Use(const Mesh* mesh) const;
        void Clear();

        static Material* LoadFromFile(const std::string& path);
        static Material* CreateEmptyMaterial(const std::string& shaderPath, const std::string& name = "Unnamed Material");

        static void SetGlobalIntValue(const std::string& paramName, int value);
        static void SetGlobalBoolValue(const std::string& paramName, bool value);
        static void SetGlobalFloatValue(const std::string& paramName, float value);
        static void SetGlobalMat4Value(const std::string& paramName, const glm::mat4& value);
        static void SetGlobalTextureValue(const std::string& paramName, Texture* value);
        static void SetGlobalVector4Value(const std::string& paramName, const glm::vec4& value);
        static void SetGlobalFloatArrValue(const std::string& paramName, const float *value, int count);
        static void ClearAllGlobalValues();
        static void ReleaseStaticRes();

    private:
        template<typename T>
        static bool FindParam(
            const std::string& paramName,
            const std::unordered_map<std::string, T>& localParam,
            const std::unordered_map<std::string, T>& globalParam,
            T& result)
        {
            auto it = localParam.find(paramName);
            if (it != localParam.end())
            {
                result = it->second;
                return true;
            }

            it = globalParam.find(paramName);
            if (it != globalParam.end())
            {
                result = it->second;
                return true;
            }

            return false;
        }
        
        static Material* s_globalMaterial;
    };
}
