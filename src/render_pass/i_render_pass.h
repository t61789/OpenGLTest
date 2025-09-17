#pragma once
#include <string>

namespace op
{
    class RenderContext;

    class IRenderPass
    {
    public:
        IRenderPass() = default;
        virtual ~IRenderPass() = default;
        IRenderPass(const IRenderPass& other) = delete;
        IRenderPass(IRenderPass&& other) noexcept = delete;
        IRenderPass& operator=(const IRenderPass& other) = delete;
        IRenderPass& operator=(IRenderPass&& other) noexcept = delete;
        
        virtual std::string GetName() = 0;
        virtual void Execute() = 0;
    };
}
