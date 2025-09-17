#pragma once
#include "i_render_pass.h"

namespace op
{
    class TestDrawPass : public IRenderPass
    {
    public:
        TestDrawPass(RenderContext* renderContext);
        ~TestDrawPass() override;

        std::string GetName() override;
        void Execute() override;

    private:
        GLuint m_vao;
        GLuint m_vbo;
        GLuint m_ebo;

        GLuint m_indirectBuffer;

        Shader* m_shader;
    };
}
