#pragma once
#include "render_pass.h"
#include "glad/glad.h"

namespace op
{
    class RenderScenePass : public RenderPass
    {
    public:

        GLuint m_buffer;
        
        explicit RenderScenePass(RenderContext* renderContext);
        ~RenderScenePass() override;
        std::string GetName() override;
        void Execute() override;
    };
}
