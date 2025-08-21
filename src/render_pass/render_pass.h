#pragma once
#include <string>

#include "render_context.h"
#include "render/per_object_buffer.h"

namespace op
{
    class RenderContext;

    class RenderPass
    {
    public:
        explicit RenderPass(RenderContext* renderContext) {this->m_renderContext = renderContext;}
        virtual ~RenderPass() = default;
        virtual std::string GetName() = 0;
        virtual void Execute() = 0;

    protected:
        RenderContext* m_renderContext = nullptr;
        PerObjectStruct m_submitBuffer;
    };
}
