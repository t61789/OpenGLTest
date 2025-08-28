#pragma once
#include "render_pass.h"

namespace op
{
    class TestDrawPass : public RenderPass
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
