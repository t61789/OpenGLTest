#pragma once
#include <cstdint>

#include "utils.h"

namespace op
{
    enum class CullMode : uint8_t
    {
        NONE = 0,
        FRONT = 1,
        BACK = 2,
        ALL = 3
    };

    enum class BlendMode : uint8_t
    {
        NONE = 0,
        BLEND = 1,
        ADD = 2
    };

    class Shader;
    
    class RenderState : public Singleton<RenderState>
    {
    public:
        RenderState();
        ~RenderState();

        void SetCullMode(CullMode cullMode);
        CullMode GetCullMode() { return m_cullMode; }
        static CullMode CullModeFromStr(const std::string& str);
        
        void SetBlendMode(BlendMode blendMode);
        BlendMode GetBlendMode() { return m_blendMode; }
        static BlendMode BlendModeFromStr(const std::string& str);

        uint32_t GetShader();
        uint32_t GetVertexArray();
        uint32_t GetBuffer(uint32_t target);
        uint32_t GetBufferBase(uint32_t target, uint32_t slot);
        
        bool BindShader(GLuint shader);
        bool BindVertexArray(GLuint vao);
        bool BindBuffer(uint32_t target, uint32_t buffer);
        bool BindBufferBase(uint32_t target, uint32_t slot, uint32_t buffer);
        bool DeleteBuffer(uint32_t target, uint32_t buffer);

        void SetAllDirty();
        void CheckGlStateMachine();

        static uint32_t GetGlShader();
        static uint32_t GetGlVertexArray();
        static uint32_t GetGlBuffer(uint32_t target);
        static uint32_t GetGlBufferBase(uint32_t target, uint32_t slot);

    private:
        struct GlBufferBaseInfo
        {
            bool dirty = true;
            uint32_t slot = GL_NONE;
            uint32_t buffer = GL_NONE;

            explicit GlBufferBaseInfo(const uint32_t slot)
            {
                this->slot = slot;   
            }
        };

        struct GlBufferInfo
        {
            bool dirty = true;
            uint32_t target = GL_NONE;
            uint32_t buffer = GL_NONE;
            std::vector<GlBufferBaseInfo> baseInfo;

            explicit GlBufferInfo(const uint32_t target)
            {
                this->target = target;
                this->baseInfo.reserve(MAX_SUPPORT_SLOTS);
                for (uint32_t i = 0; i < MAX_SUPPORT_SLOTS; ++i)
                {
                    this->baseInfo.emplace_back(i);
                }
            }
        };

        struct GlShaderInfo
        {
            bool dirty = true;
            GLuint shader = GL_NONE;
        };

        struct GlVertexArrayInfo
        {
            bool dirty = true;
            GLuint vao = GL_NONE;
        };
        
        CullMode m_cullMode = CullMode::NONE; // dirty
        BlendMode m_blendMode = BlendMode::NONE;

        GlShaderInfo m_glShader;
        GlVertexArrayInfo m_glVertexArray;

        std::vector<GlBufferInfo> m_glBuffers;

        EventHandler m_onFrameEndHandler;

        GlBufferInfo* GetBufferInfo(uint32_t target);
    };

    static RenderState* GetRS()
    {
        return RenderState::Ins();
    }
}
