#include "gl_vertex_array.h"

#include <glad/glad.h>

#include "gl_buffer.h"
#include "gl_state.h"

namespace op
{
    GlVertexArray::GlVertexArray()
    {
        m_id = GlState::GlGenVertexArray();
    }

    GlVertexArray::~GlVertexArray()
    {
        Delete();
    }

    void GlVertexArray::Use()
    {
        assert(!m_deleted && !m_settingAttr);
        assert(m_vbo && m_ebo);

        GlState::Ins()->BindVertexArray(shared_from_this());
    }

    void GlVertexArray::StopUse()
    {
        assert(!m_deleted && !m_settingAttr);
        assert(GlState::Ins()->GetVertexArray().get() == this);

        GlState::Ins()->UnBindVertexArray();
    }

    void GlVertexArray::Delete()
    {
        assert(!m_deleted && !m_settingAttr);

        if (GlState::Ins()->GetVertexArray().get() == this)
        {
            GlState::Ins()->UnBindVertexArray();
        }

        GlState::GlDeleteVertexArray(m_id);
        m_vbo.reset();
        m_ebo.reset();
        m_deleted = true;
    }

    void GlVertexArray::StartSetting()
    {
        assert(!m_deleted && !m_settingAttr);

        m_settingAttr = true;
        
        GlState::Ins()->UseGlResource(shared_from_this());
        GlState::Ins()->BindVertexArray(shared_from_this());
    }

    void GlVertexArray::EndSetting()
    {
        assert(!m_deleted && m_settingAttr);
        
        m_settingAttr = false;

        GlState::Ins()->EndUseGlResource(shared_from_this());
        GlState::Ins()->UnBindVertexArray();
    }

    void GlVertexArray::BindVbo(const std::shared_ptr<GlBuffer>& vbo)
    {
        assert(!m_deleted && m_settingAttr);
        assert(vbo->GetType() == GL_ARRAY_BUFFER);
        
        m_vbo = vbo;

        GlState::Ins()->BindBuffer(vbo);
    }

    void GlVertexArray::BindEbo(const std::shared_ptr<GlBuffer>& ebo)
    {
        assert(!m_deleted && m_settingAttr);
        assert(ebo->GetType() == GL_ELEMENT_ARRAY_BUFFER);

        m_ebo = ebo;

        GlState::Ins()->BindBuffer(ebo);
    }

    void GlVertexArray::SetAttrEnable(const uint32_t index, const bool enable)
    {
        assert(!m_deleted && m_settingAttr);
        assert(m_vbo);

        GlState::GlSetVertAttrEnable(index, enable);
    }

    void GlVertexArray::SetAttr(
        const VertexAttr attr,
        const uint32_t vertexDataStrideB,
        const uint32_t vertexDataOffsetB)
    {
        assert(!m_deleted && m_settingAttr);
        assert(m_vbo);

        auto index = find_index(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, attr);
        auto& attrInfo = VERTEX_ATTR_DEFINES[index];
        GlState::GlSetVertAttrLayout(
            index,
            attrInfo.strideF,
            GL_FLOAT,
            GL_FALSE,
            vertexDataStrideB,
            vertexDataOffsetB);
    }
}
