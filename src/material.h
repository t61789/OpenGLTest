#pragma once
#include <string>
#include <unordered_map>

#include "blend_mode.h"
#include "cull_mode.h"
#include "shared_object.h"
#include "string_handle.h"
#include "math/math.h"

namespace op
{
    class Shader;
    class Texture;
    class Mesh;

    class MaterialTextureValue
    {
    public:
        Texture* texture = nullptr;
        size_t texelNameId = 0;
    };

    class Material : public SharedObject
    {
    public:
        std::string name = "Unnamed Material";

        Shader* shader = nullptr;

        CullMode cullMode = CullMode::Back;
        BlendMode blendMode = BlendMode::None;

        std::unordered_map<size_t, int> intValues;
        std::unordered_map<size_t, bool> boolValues;
        std::unordered_map<size_t, float> floatValues;
        std::unordered_map<size_t, Matrix4x4> mat4Values;
        std::unordered_map<size_t, Vec4> vec4Values;
        std::unordered_map<size_t, std::vector<float>*> floatArrValues;
        std::unordered_map<size_t, MaterialTextureValue> textureValues;
        
        Material() = default;
        Material(const std::string& name);
        ~Material() override;

        void SetIntValue(const StringHandle& paramName, int value);
        void SetBoolValue(const StringHandle& paramName, bool value);
        void SetFloatValue(const StringHandle& paramName, float value);
        void SetMat4Value(const StringHandle& paramName, const Matrix4x4& value);
        void SetVector4Value(const StringHandle& paramName, const Vec4& value);
        void SetFloatArrValue(const StringHandle& paramName, const float *value, int count);
        void SetTextureValue(const StringHandle& paramName, Texture* value);

        void FillParams(const Shader* targetShader) const;
        void Use(const Mesh* mesh) const;
        void Clear();

        static Material* LoadFromFile(const std::string& path);
        static Material* CreateEmptyMaterial(const std::string& shaderPath, const std::string& name = "Unnamed Material");
        static Material* CreateEmptyMaterial(Shader* shader, const std::string& name = "Unnamed Material");

        static void SetGlobalIntValue(const StringHandle& paramName, int value);
        static void SetGlobalBoolValue(const StringHandle& paramName, bool value);
        static void SetGlobalFloatValue(const StringHandle& paramName, float value);
        static void SetGlobalMat4Value(const StringHandle& paramName, const Matrix4x4& value);
        static void SetGlobalVector4Value(const StringHandle& paramName, const Vec4& value);
        static void SetGlobalFloatArrValue(const StringHandle& paramName, const float *value, int count);
        static void SetGlobalTextureValue(const std::string& paramName, Texture* value);
        static void ClearAllGlobalValues();
        static void ReleaseStaticRes();

    private:
        template<typename K, typename T>
        static bool FindParam(
            const K nameId,
            const std::unordered_map<K, T>& localParam,
            const std::unordered_map<K, T>& globalParam,
            T& result)
        {
            auto it = localParam.find(nameId);
            if (it != localParam.end())
            {
                result = it->second;
                return true;
            }

            it = globalParam.find(nameId);
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
