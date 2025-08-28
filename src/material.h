#pragma once

#include <unordered_set>
#include <glad/glad.h>

#include "game_resource.h"
#include "render_context.h"
#include "shader.h"
#include "shared_object.h"
#include "math/vec.h"
#include "render/cbuffer.h"

namespace op
{
    class Material : public SharedObject
    {
    public:
        CullMode cullMode = CullMode::BACK;
        BlendMode blendMode = BlendMode::NONE;
        
        Material() = default;
        ~Material() override;

        inline void Set(size_t nameId, bool value);
        inline void Set(size_t nameId, int value);
        inline void Set(size_t nameId, float value);
        inline void Set(size_t nameId, const Vec4& value);
        inline void Set(size_t nameId, const Matrix4x4& value);
        inline void Set(size_t nameId, Texture* value);
        inline void Set(size_t nameId, const float* value, uint32_t count);
        
        inline bool GetBool(size_t nameId);
        inline int GetInt(size_t nameId);
        inline float GetFloat(size_t nameId);
        inline Vec4 GetVec4(size_t nameId);
        inline Matrix4x4 GetMatrix4x4(size_t nameId);
        inline Texture* GetTexture(size_t nameId);
        inline bool GetFloatArr(size_t nameId, float* dst, const uint32_t& count);

        void BindShader(Shader* shader);
        Shader* GetShader() const { return m_shader; }
        void CreateCBuffer(CBufferLayout* cbufferLayout);
        bool HasCBuffer() { return m_cbuffer != nullptr; }
        void UseCBuffer();

        static Material* LoadFromFile(const std::string& path);
        static Material* CreateFromShader(const std::string& path);
        
    private:
        struct ValueInfo
        {
            size_t nameId;
            uint32_t byteCount;
            uint8_t* data = nullptr;
        };

        std::vector<IndexObj<Texture*>> m_textures;
        std::vector<ValueInfo> m_values;
        std::unordered_set<size_t> m_dirtyValues;

        Shader* m_shader = nullptr;
        CBuffer* m_cbuffer = nullptr;

        void SyncCBuffer(bool force = false);
        void BindUbo();
        void SetAllValuesDirty();

        void OnFrameEnd();

        void SetImp(size_t nameId, const void* src, uint32_t byteCount);
        bool GetImp(size_t nameId, void* dst, uint32_t byteCount);
    };
    
    inline void Material::Set(const size_t nameId, const bool value)
    {
        SetImp(nameId, &value, sizeof(bool));
    }

    inline void Material::Set(const size_t nameId, const int value)
    {
        SetImp(nameId, &value, sizeof(int));
    }
    
    inline void Material::Set(const size_t nameId, const float value)
    {
        SetImp(nameId, &value, sizeof(float));
    }

    inline void Material::Set(const size_t nameId, const Vec4& value)
    {
        SetImp(nameId, &value.x, sizeof(float) * 4);
    }

    inline void Material::Set(const size_t nameId, const Matrix4x4& value)
    {
        SetImp(nameId, value.GetReadOnlyData(), sizeof(float) * 16);
    }
    
    inline void Material::Set(const size_t nameId, Texture* value)
    {
        auto textureData = find(m_textures, nameId);
        if (!textureData)
        {
            m_textures.push_back({nameId, nullptr});
            textureData = &m_textures.back().obj;
        }
        else if (*textureData)
        {
            DECREF(*textureData)
        }

        *textureData = value;
        if (value)
        {
            INCREF(value)
        }
    }

    inline void Material::Set(const size_t nameId, const float* value, const uint32_t count)
    {
        SetImp(nameId, value, sizeof(float) * count);
    }
    
    inline bool Material::GetBool(const size_t nameId)
    {
        bool result;
        GetImp(nameId, &result, sizeof(bool));
        return result;
    }
    
    inline int Material::GetInt(const size_t nameId)
    {
        int result;
        GetImp(nameId, &result, sizeof(int));
        return result;
    }
    
    inline float Material::GetFloat(const size_t nameId)
    {
        float result;
        GetImp(nameId, &result, sizeof(float));
        return result;
    }

    inline Vec4 Material::GetVec4(const size_t nameId)
    {
        Vec4 result;
        GetImp(nameId, &result, sizeof(float) * 4);
        return result;
    }

    inline Matrix4x4 Material::GetMatrix4x4(const size_t nameId)
    {
        Matrix4x4 result;
        GetImp(nameId, result.GetData(), sizeof(float) * 16);
        return result;
    }
    
    inline Texture* Material::GetTexture(const size_t nameId)
    {
        auto textureData = find(m_textures, nameId);
        if (textureData != nullptr)
        {
            return *textureData;
        }

        auto globalMat = GET_GLOBAL_CBUFFER;
        if (globalMat != this)
        {
            return globalMat->GetTexture(nameId);
        }

        return nullptr;
    }
    
    inline bool Material::GetFloatArr(const size_t nameId, float* dst, const uint32_t& count)
    {
        return GetImp(nameId, dst, count * sizeof(float));
    }
    
    inline void Material::SetImp(const size_t nameId, const void* src, const uint32_t byteCount)
    {
        auto valueInfo = find(m_values, &ValueInfo::nameId, nameId);
        if (!valueInfo)
        {
            m_values.push_back({ nameId, byteCount, new uint8_t[byteCount] });
            valueInfo = &m_values.back();
        }

        if (valueInfo->byteCount != byteCount)
        {
            delete[] valueInfo->data;
            valueInfo->data = new uint8_t[byteCount];
            valueInfo->byteCount = byteCount;
            m_dirtyValues.insert(nameId);
        }
        
        if (memcmp(valueInfo->data, src, byteCount) != 0)
        {
            memcpy(valueInfo->data, src, byteCount);
            m_dirtyValues.insert(nameId);
        }
    }

    inline bool Material::GetImp(const size_t nameId, void* dst, const uint32_t byteCount)
    {
        auto valueInfo = find(m_values, &ValueInfo::nameId, nameId);
        if (!valueInfo)
        {
            return false;
        }

        if (valueInfo->byteCount != byteCount)
        {
            THROW_ERRORF("获取参数类型不匹配，材质中：%u，期望：%u", valueInfo->byteCount, byteCount)
        }

        memcpy(dst, valueInfo->data, byteCount);
        return true;
    }
}
