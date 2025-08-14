#pragma once

#include <unordered_set>
#include <glad/glad.h>

#include "blend_mode.h"
#include "cull_mode.h"
#include "render_context.h"
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

        void Use(const Mesh* mesh = nullptr, const RenderContext* renderContext = nullptr);

        inline void Set(size_t nameId, float value);
        inline void Set(size_t nameId, const Vec4& value);
        inline void Set(size_t nameId, const Matrix4x4& value);
        inline void Set(size_t nameId, Texture* value);
        
        inline float GetFloat(size_t nameId);
        inline Vec4 GetVec4(size_t nameId);
        inline Matrix4x4 GetMatrix4x4(size_t nameId);
        inline Texture* GetTexture(size_t nameId);

        void SetShader(Shader* shader);
        void CreateCBuffer(CBufferLayout* cbufferLayout);
        bool HasCBuffer() { return m_cbuffer != nullptr; }

        static MaterialNew* LoadFromFile(const std::string& path);
        inline static MaterialNew* GetGlobalMat();
        inline static void ReleaseGlobalMat();
        
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

        static MaterialNew* s_globalMat;

        void SyncCBuffer(bool force = false);
        void BindUbo();
        void SetAllValuesDirty();
        void ApplyTextures(const RenderContext* renderContext = nullptr);

        void OnFrameEnd();

        template <typename T>
        uint32_t GetTypeSize();
        template <typename T>
        void SetInfo(size_t nameId);
        template <typename Ft, typename T>
        void Set(size_t nameId, T Ft::* unionField, std::unordered_map<size_t, Ft> MaterialNew::* dataField, const T& value);
        template <typename Ft, typename T>
        T Get(size_t nameId, T Ft::* unionField, std::unordered_map<size_t, Ft> MaterialNew::* dataField);
        void* Get(size_t nameId, uint32_t dataSize);
        template <typename Ft>
        void* Get(size_t nameId, std::unordered_map<size_t, Ft> MaterialNew::* dataField);
    };
    
    inline void MaterialNew::Set(const size_t nameId, const float value)
    {
        Set(nameId, &Value4::f, &MaterialNew::m_v4Values, value);
    }

    inline void MaterialNew::Set(const size_t nameId, const Vec4& value)
    {
        Set(nameId, &Value16::v4, &MaterialNew::m_v16Values, value);
    }

    inline void MaterialNew::Set(const size_t nameId, const Matrix4x4& value)
    {
        SetInfo<Matrix4x4>(nameId);
        auto& dstValue = m_v64Values[nameId];
        if (!dstValue.m4.RudeCmp(value))
        {
            dstValue.m4 = value;
            m_dirtyValues.insert(nameId);
        }
    }
    
    inline void MaterialNew::Set(const size_t nameId, Texture* value)
    {
        auto it = m_textures.find(nameId);
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
        
        m_textures[nameId] = value;
        INCREF(value);
    }
    
    inline float MaterialNew::GetFloat(const size_t nameId)
    {
        return Get(nameId, &Value4::f, &MaterialNew::m_v4Values);
    }

    inline Vec4 MaterialNew::GetVec4(const size_t nameId)
    {
        return Get(nameId, &Value16::v4, &MaterialNew::m_v16Values);
    }

    inline Matrix4x4 MaterialNew::GetMatrix4x4(const size_t nameId)
    {
        return Get(nameId, &Value64::m4, &MaterialNew::m_v64Values);
    }

    inline Texture* MaterialNew::GetTexture(const size_t nameId)
    {
        auto it = m_textures.find(nameId);
        if (it != m_textures.end())
        {
            return it->second;
        }

        auto globalMat = GetGlobalMat();
        if (this == globalMat)
        {
            return nullptr;
        }

        return globalMat->GetTexture(nameId);
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
            
            m_valueInfos[nameId] = valueInfo;
            return;
        }

        if (it->second.size != size)
        {
            throw std::runtime_error("参数类型不匹配");
        }
    }

    template <typename Ft, typename T>
    void MaterialNew::Set(size_t nameId, T Ft::* unionField, std::unordered_map<size_t, Ft> MaterialNew::* dataField, const T& value)
    {
        SetInfo<T>(nameId);
        auto& values = this->*dataField;
        auto& dstValue = values[nameId];
        if (dstValue.*unionField != value)
        {
            dstValue.*unionField = value;
            m_dirtyValues.insert(nameId);
        }
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

    MaterialNew* MaterialNew::GetGlobalMat()
    {
        if (!s_globalMat)
        {
            s_globalMat = new MaterialNew();
        }

        return s_globalMat;
    }
    
    void MaterialNew::ReleaseGlobalMat()
    {
        delete s_globalMat;
        s_globalMat = nullptr;
    }
}
