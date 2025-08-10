#pragma once

#include <unordered_set>
#include <glad/glad.h>

#include "blend_mode.h"
#include "cull_mode.h"
#include "shader.h"
#include "shared_object.h"
#include "math/vec.h"
#include "render/cbuffer.h"

namespace op
{
    class MaterialNew : public SharedObject
    {
    public:
        CullMode cullMode = CullMode::Back;
        BlendMode blendMode = BlendMode::None;
        
        explicit MaterialNew();
        ~MaterialNew() override;

        void Use(const Mesh* mesh = nullptr);

        inline void Set(const StringHandle& name, float value);
        inline void Set(const StringHandle& name, const Vec4& value);
        inline void Set(const StringHandle& name, const Matrix4x4& value);
        inline void Set(const StringHandle& name, Texture* value);
        
        inline float GetFloat(const StringHandle& name);
        inline Vec4 GetVec4(const StringHandle& name);
        inline Matrix4x4 GetMatrix4x4(const StringHandle& name);

        void SetShader(Shader* shader);
        void CreateCBuffer(CBufferLayout* cbufferLayout);
        bool HasCBuffer() { return m_cbuffer != nullptr; }

        static MaterialNew* LoadFromFile(const std::string& path);
        
    private:

        union Value4
        {
            float f;
            uint32_t ui;
            int32_t i;
        };

        union Value16
        {
            Vec4 v4;

            Value16() { new (&v4) Vec4(); }
            ~Value16() { v4.~Vec4(); }
        };

        union Value64
        {
            Matrix4x4 m4;

            Value64() { new (&m4) Matrix4x4(); }
            ~Value64() { m4.~Matrix4x4(); }
        };

        struct ValueInfo
        {
            uint32_t size;
        };

        std::unordered_map<size_t, Value4> m_v4Values;
        std::unordered_map<size_t, Value16> m_v16Values;
        std::unordered_map<size_t, Value64> m_v64Values;
        std::unordered_map<size_t, ValueInfo> m_valueInfos;
        std::unordered_map<size_t, Texture*> m_textures;

        std::unordered_set<size_t> m_dirtyValues;

        EventHandler m_onFrameEndHandler;

        Shader* m_shader = nullptr;
        CBuffer* m_cbuffer = nullptr;

        void SyncCBuffer(bool force = false);
        void BindUbo();
        void SetAllValuesDirty();
        void ApplyTextures();

        void OnFrameEnd();

        template <typename T>
        uint32_t GetTypeSize();
        template <typename T>
        void SetInfo(size_t nameId);
        template <typename Ft, typename T>
        void Set(size_t nameId, T Ft::* unionField, std::unordered_map<size_t, Ft> MaterialNew::* dataField, T value);
        template <typename Ft, typename T>
        T Get(size_t nameId, T Ft::* unionField, std::unordered_map<size_t, Ft> MaterialNew::* dataField);
        void* Get(size_t nameId, uint32_t dataSize);
        template <typename Ft>
        void* Get(size_t nameId, std::unordered_map<size_t, Ft> MaterialNew::* dataField);
    };
    
    inline void MaterialNew::Set(const StringHandle& name, const float value)
    {
        Set(name.Hash(), &Value4::f, &MaterialNew::m_v4Values, value);
    }

    inline void MaterialNew::Set(const StringHandle& name, const Vec4& value)
    {
        Set(name.Hash(), &Value16::v4, &MaterialNew::m_v16Values, value);
    }

    inline void MaterialNew::Set(const StringHandle& name, const Matrix4x4& value)
    {
        Set(name.Hash(), &Value64::m4, &MaterialNew::m_v64Values, value);
    }
    
    inline void MaterialNew::Set(const StringHandle& name, Texture* value)
    {
        auto it = m_textures.find(name.Hash());
        if (it != m_textures.end())
        {
            if (it->second == value)
            {
                return;
            }
            
            DECREF(it->second);

            if (!value)
            {
                m_textures.erase(it);
                return;
            }
        }
        
        m_textures[name.Hash()] = value;
        INCREF(value);
    }

    inline float MaterialNew::GetFloat(const StringHandle& name)
    {
        return Get(name.Hash(), &Value4::f, &MaterialNew::m_v4Values);
    }

    inline Vec4 MaterialNew::GetVec4(const StringHandle& name)
    {
        return Get(name.Hash(), &Value16::v4, &MaterialNew::m_v16Values);
    }

    inline Matrix4x4 MaterialNew::GetMatrix4x4(const StringHandle& name)
    {
        return Get(name.Hash(), &Value64::m4, &MaterialNew::m_v64Values);
    }

    template <typename T>
    uint32_t MaterialNew::GetTypeSize()
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return 4;
        }
        else if constexpr (std::is_same_v<T, Vec4>)
        {
            return 16;
        }
        else if constexpr (std::is_same_v<T, Matrix4x4>)
        {
            return 64;
        }
        else
        {
            throw std::runtime_error("Unsupported type");
        }
    }
    
    template <typename T>
    void MaterialNew::SetInfo(const size_t nameId)
    {
        auto size = GetTypeSize<T>();
        auto it = m_valueInfos.find(nameId);
        if (it == m_valueInfos.end())
        {
            ValueInfo valueInfo;
            valueInfo.size = size;
            
            m_valueInfos[nameId] = std::move(valueInfo);
            return;
        }

        if (it->second.size != size)
        {
            throw std::runtime_error("参数类型不匹配");
        }
    }

    template <typename Ft, typename T>
    void MaterialNew::Set(size_t nameId, T Ft::* unionField, std::unordered_map<size_t, Ft> MaterialNew::* dataField, T value)
    {
        SetInfo<T>(nameId);
        auto& values = this->*dataField;
        auto& dstValue = values[nameId];
        dstValue.*unionField = value;

        m_dirtyValues.insert(nameId);
    }

    template <typename Ft, typename T>
    T MaterialNew::Get(size_t nameId, T Ft::* unionField, std::unordered_map<size_t, Ft> MaterialNew::* dataField)
    {
        if (auto data = Get(nameId, dataField))
        {
            return static_cast<Ft*>(data)->*unionField;
        }
        
        return {};
    }

    inline void* MaterialNew::Get(const size_t nameId, const uint32_t dataSize)
    {
        switch (dataSize)
        {
        case 4:
            return Get(nameId, &MaterialNew::m_v4Values);
        case 16:
            return Get(nameId, &MaterialNew::m_v16Values);
        case 64:
            return Get(nameId, &MaterialNew::m_v64Values);
        default:
            return nullptr;
        }
    }

    template <typename Ft>
    void* MaterialNew::Get(size_t nameId, std::unordered_map<size_t, Ft> MaterialNew::* dataField)
    {
        auto& data = this->*dataField;
        if (auto it = data.find(nameId); it != data.end())
        {
            return static_cast<void*>(&it->second);
        }

        return nullptr;
    }
}
