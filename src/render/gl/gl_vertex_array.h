#pragma once
#include <cstdint>
#include <memory>

#include "const.h"
#include "i_gl_resource.h"

namespace op
{
    class GlBuffer;
    
    class GlVertexArray : public IGlResource, public std::enable_shared_from_this<GlVertexArray>
    {
    public:
        GlVertexArray();
        ~GlVertexArray();
        
        uint32_t GetId() const { return m_id;}
        bool IsDeleted() const { return m_deleted;}
        bool IsSettingAttr() const { return m_settingAttr;}
        crsp<GlBuffer> GetVbo() const { return m_vbo;}
        crsp<GlBuffer> GetEbo() const { return m_ebo;}

        void Use();
        void StopUse();
        void Delete();

        void StartSetting();
        void EndSetting();
        void BindVbo(const std::shared_ptr<GlBuffer>& vbo);
        void BindEbo(const std::shared_ptr<GlBuffer>& ebo);
        void SetAttrEnable(uint32_t index, bool enable);
        void SetAttr(VertexAttr attr, uint32_t vertexDataStrideB, uint32_t vertexDataOffsetB);

    private:
        uint32_t m_id;
        bool m_deleted = false;
        bool m_settingAttr = false;

        std::shared_ptr<GlBuffer> m_vbo;
        std::shared_ptr<GlBuffer> m_ebo;
    };
}
