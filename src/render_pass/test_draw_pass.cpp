#include "test_draw_pass.h"

#include "game_resource.h"
#include "material.h"
#include "render_target.h"
#include "shader.h"

namespace op
{
    struct IndirectBuffer
    {
        uint32_t count;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t baseVertex;
        uint32_t baseInstance;
    };
    
    TestDrawPass::TestDrawPass(RenderContext* renderContext): RenderPass(renderContext)
    {
        struct Vertex
        {
            float x;
            float y;
            float z;
            float w;
            float u;
            float v;
        };

        Vertex vertices[] =
        {
            {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
            {0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f},
            {0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f},
            {-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
            
            {-0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f},
            {0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f},
            {0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f},
            {-0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f}
        };

        int indices[] =
        {
            0, 1, 2,
            2, 3, 0,
            
            0, 1, 2,
            2, 3, 0,
        };
        
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        GetRS()->BindVertexArray(m_vao);
        GetRS()->BindBuffer(GL_ARRAY_BUFFER, m_vbo);
        auto aa = sizeof(vertices);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(vertices)), vertices, GL_STATIC_DRAW);
        GetRS()->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(indices)), indices, GL_STATIC_DRAW);
        GL_CHECK_ERROR(456)

        glEnableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glDisableVertexAttribArray(4);

        GL_CHECK_ERROR(123)
        
        glVertexAttribPointer(
            0,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<const void*>(0));

        GL_CHECK_ERROR(33)
        
        glVertexAttribPointer(
            3,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<const void*>(sizeof(float) * 4));

        IndirectBuffer indirectBuffer[] =
        {
            {6, 1, 0, 0, 0},
            {6, 1, 6, 4, 1},
        };

        glGenBuffers(1, &m_indirectBuffer);
        GetRS()->BindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(indirectBuffer), indirectBuffer, GL_STATIC_DRAW);

        m_shader = Shader::LoadFromFile("shaders/indirect_test.shader");
    }

    TestDrawPass::~TestDrawPass()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
        glDeleteBuffers(1, &m_indirectBuffer);
    }

    std::string TestDrawPass::GetName()
    {
        return "TestDrawPass";
    }

    void TestDrawPass::Execute()
    {
        RenderTarget::Get(*m_renderContext->gBufferDesc)->Use();
        
        GetRS()->SetShader(m_shader->glShaderId);
        GetGR()->GetPredefinedMaterial(GLOBAL_CBUFFER)->UseCBuffer();
        GetGR()->GetPredefinedMaterial(PER_VIEW_CBUFFER)->UseCBuffer();
        GetRS()->BindVertexArray(m_vao);
        GetRS()->BindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 2, 0);
    }
}
