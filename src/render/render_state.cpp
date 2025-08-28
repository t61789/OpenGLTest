#include "render_state.h"

#include "game_resource.h"
#include "shared_object.h"

namespace op
{
    RenderState::RenderState()
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        m_onFrameEndHandler = GetGR()->onFrameEnd.Add(this, &RenderState::SetAllDirty);
    }

    RenderState::~RenderState()
    {
        GetGR()->onFrameEnd.Remove(m_onFrameEndHandler);
        
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

    void RenderState::SetCullMode(const CullMode cullMode)
    {
        if (m_cullMode == cullMode)
        {
            return;
        }

        if (m_cullMode == CullMode::NONE && cullMode != CullMode::NONE)
        {
            glEnable(GL_CULL_FACE);
        }
        else if (m_cullMode != CullMode::NONE && cullMode == CullMode::NONE)
        {
            glDisable(GL_CULL_FACE);
        }

        if (cullMode == CullMode::FRONT)
        {
            glCullFace(GL_FRONT);
        }
        else if (cullMode == CullMode::BACK)
        {
            glCullFace(GL_BACK);
        }
        else if (cullMode == CullMode::ALL)
        {
            glCullFace(GL_FRONT_AND_BACK);
        }

        m_cullMode = cullMode;
    }

    CullMode RenderState::CullModeFromStr(const std::string& str)
    {
        static const std::unordered_map<std::string, CullMode> CULL_MODE_MAP =
        {
            {"None", CullMode::NONE},
            {"Front", CullMode::FRONT},
            {"Back", CullMode::BACK},
            {"All", CullMode::ALL}
        };

        auto it = CULL_MODE_MAP.find(str);
        if (it != CULL_MODE_MAP.end())
        {
            return it->second;
        }
        return CullMode::NONE;
    }

    void RenderState::SetBlendMode(const BlendMode blendMode)
    {
        if (m_blendMode == blendMode)
        {
            return;
        }

        if (m_blendMode != BlendMode::NONE && blendMode == BlendMode::NONE)
        {
            glDisable(GL_BLEND);
        }
        else if (m_blendMode == BlendMode::NONE && blendMode != BlendMode::NONE)
        {
            glEnable(GL_BLEND);
        }

        if (blendMode == BlendMode::BLEND)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else if (blendMode == BlendMode::ADD)
        {
            glBlendFunc(GL_ONE, GL_ONE);
        }

        m_blendMode = blendMode;
    }

    BlendMode RenderState::BlendModeFromStr(const std::string& str)
    {
        static const std::unordered_map<std::string, BlendMode> BLEND_MODE_MAP =
        {
            {"None", BlendMode::NONE},
            {"Blend", BlendMode::BLEND},
            {"Add", BlendMode::ADD}
        };

        auto it = BLEND_MODE_MAP.find(str);
        if (it != BLEND_MODE_MAP.end())
        {
            return it->second;
        }
        return BlendMode::NONE;
    }

    uint32_t RenderState::GetShader()
    {
        return m_glShader.shader;
    }

    uint32_t RenderState::GetVertexArray()
    {
        return m_glVertexArray.vao;
    }

    uint32_t RenderState::GetBuffer(const uint32_t target)
    {
        auto bufferInfo = GetBufferInfo(target);
        return bufferInfo->buffer;
    }

    uint32_t RenderState::GetBufferBase(const uint32_t target, const uint32_t slot)
    {
        auto bufferInfo = GetBufferInfo(target);
        return bufferInfo->baseInfo[slot].buffer;
    }
    
    bool RenderState::BindShader(const GLuint shader)
    {
        if (!m_glShader.dirty && m_glShader.shader == shader)
        {
            return false;
        }
        m_glShader.dirty = false;
        m_glShader.shader = shader;

        glUseProgram(shader);
        return true;
    }

    bool RenderState::BindVertexArray(const GLuint vao)
    {
        if (!m_glVertexArray.dirty && m_glVertexArray.vao == vao)
        {
            return false;
        }
        m_glVertexArray.dirty = false;
        m_glVertexArray.vao = vao;
        
        GetBufferInfo(GL_ELEMENT_ARRAY_BUFFER)->dirty = true;

        glBindVertexArray(vao);

        return true;
    }
    
    bool RenderState::BindBuffer(const uint32_t target, const uint32_t buffer)
    {
        auto glBufferInfo = GetBufferInfo(target);
        if (!glBufferInfo->dirty && glBufferInfo->buffer == buffer)
        {
            return false;
        }
        glBufferInfo->dirty = false;
        glBufferInfo->buffer = buffer;

        glBindBuffer(target, buffer);
        
        GL_CHECK_ERROR(绑定Buffer)
        
        return true;
    }

    bool RenderState::BindBufferBase(const uint32_t target, const uint32_t slot, const uint32_t buffer)
    {
        auto glBufferInfo = GetBufferInfo(target);
        auto glBufferBaseInfo = &glBufferInfo->baseInfo[slot];
        if (!glBufferBaseInfo->dirty && glBufferBaseInfo->buffer == buffer)
        {
            return false;
        }
        glBufferBaseInfo->dirty = false;
        glBufferBaseInfo->buffer = buffer;

        glBindBufferBase(target, slot, buffer);
        
        GL_CHECK_ERROR(绑定BufferBase)
        
        return true;
    }

    bool RenderState::DeleteBuffer(const uint32_t target, const uint32_t buffer)
    {
        auto glBufferInfo = GetBufferInfo(target);
        glBufferInfo->dirty = true;
        if (glBufferInfo->buffer == buffer)
        {
            glBufferInfo->buffer = GL_NONE;
        }

        glDeleteBuffers(1, &buffer);

        return true;
    }

    uint32_t RenderState::GetGlShader()
    {
        int result;
        glGetIntegerv(GL_CURRENT_PROGRAM, &result);
        return result;
    }

    uint32_t RenderState::GetGlVertexArray()
    {
        int result;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &result);
        return result;
    }

    uint32_t RenderState::GetGlBuffer(const uint32_t target)
    {
        int result;
        glGetIntegerv(target, &result);
        return result;
    }

    uint32_t RenderState::GetGlBufferBase(const uint32_t target, const uint32_t slot)
    {
        int result;
        glGetIntegeri_v(target, slot, &result);
        return result;
    }

    void RenderState::SetAllDirty()
    {
        m_glShader.dirty = true;
        m_glVertexArray.dirty = true;
        for (auto& bufferInfo : m_glBuffers)
        {
            bufferInfo.dirty = true;
            for (auto& bufferBaseInfo : bufferInfo.baseInfo)
            {
                bufferBaseInfo.dirty = true;
            }
        }
    }

    void RenderState::CheckGlStateMachine()
    {
        return;
        static std::unordered_map<uint32_t, uint32_t> bindingEnumMapper = 
        {
            {GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING},
            {GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING},
            {GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER_BINDING},
            {GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER_BINDING},
        };
        
        if (!m_glShader.dirty)
        {
            auto glShader = GetGlShader();
            if (m_glShader.shader != glShader)
            {
                THROW_ERRORF("Shader状态不一致：cur %u, gl %u", m_glShader.shader, glShader)
            }
        }

        if (!m_glVertexArray.dirty)
        {
            auto glVertexArray = GetGlVertexArray();
            if (m_glVertexArray.vao != glVertexArray)
            {
                THROW_ERRORF("VertexArray状态不一致：cur %u, gl %u", m_glVertexArray.vao, glVertexArray)
            }
        }

        for (auto& bufferInfo : m_glBuffers)
        {
            if (!bufferInfo.dirty)
            {
                auto glBuffer = GetGlBuffer(bindingEnumMapper.at(bufferInfo.target));
                if (bufferInfo.buffer != glBuffer)
                {
                    THROW_ERRORF("Buffer状态不一致：target %u, cur %u, gl %u", bufferInfo.target, bufferInfo.buffer, glBuffer)
                }
            }

            for (auto& bufferBaseInfo : bufferInfo.baseInfo)
            {
                if (!bufferBaseInfo.dirty)
                {
                    auto glBufferBase = GetGlBufferBase(bindingEnumMapper.at(bufferInfo.target), bufferBaseInfo.slot);
                    if (bufferBaseInfo.buffer != glBufferBase)
                    {
                        THROW_ERRORF("BufferBase状态不一致：target %u, slot %u, cur %u, gl %u", bufferInfo.target, bufferBaseInfo.slot, bufferBaseInfo.buffer, glBufferBase)
                    }
                }
            }
        }

        GL_CHECK_ERROR(检查GL状态机)
    }

    RenderState::GlBufferInfo* RenderState::GetBufferInfo(const uint32_t target)
    {
        auto glBufferInfo = find(m_glBuffers, &GlBufferInfo::target, target);
        if (!glBufferInfo)
        {
            m_glBuffers.emplace_back(target);
            glBufferInfo = &m_glBuffers.back();
            glBufferInfo->target = target;
        }

        return glBufferInfo;
    }
}
