#pragma once
#include <cstdint>
#include <memory>

#include "const.h"
#include "i_gl_resource.h"

namespace op
{
    class GlBuffer;
    
    class GlVertexArray final : public IGlResource, public std::enable_shared_from_this<GlVertexArray>
    {
    public:
        GlVertexArray();
        ~GlVertexArray();
        GlVertexArray(const GlVertexArray& other) = delete;
        GlVertexArray(GlVertexArray&& other) noexcept = delete;
        GlVertexArray& operator=(const GlVertexArray& other) = delete;
        GlVertexArray& operator=(GlVertexArray&& other) noexcept = delete;

        uint32_t GetId() const { return m_id;}
        bool IsSettingAttr() const { return m_settingAttr;}
        crsp<GlBuffer> GetVbo() const { return m_vbo;}
        crsp<GlBuffer> GetEbo() const { return m_ebo;}

        void Use();
        void StopUse();

        void BindVbo(const std::shared_ptr<GlBuffer>& vbo);
        void BindEbo(const std::shared_ptr<GlBuffer>& ebo);
        void SetAttrEnable(uint32_t index, bool enable);
        void SetAttr(VertexAttr attr, uint32_t vertexDataStrideB, uint32_t vertexDataOffsetB);

        // lock_guard implements
        void lock();
        void unlock();

    private:
        uint32_t m_id;
        bool m_settingAttr = false;

        std::shared_ptr<GlBuffer> m_vbo;
        std::shared_ptr<GlBuffer> m_ebo;
        
        void StartSetting();
        void EndSetting();
    };
}
