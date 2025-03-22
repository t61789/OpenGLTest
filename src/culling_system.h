#pragma once
#include <vector>

#include "gui.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class RenderContext;
    class Bounds;
    class RenderComp;
    class Object;

    class CullingSystem : public ControlPanelUi::UiProxy
    {
    public:
        explicit CullingSystem(RenderContext* renderContext);
        void Cull();

        void DrawConsoleUi() override;

    private:
        RenderContext* m_renderContext = nullptr;
        std::vector<Bounds> m_bounds;

        bool CullOnce(const Bounds& bounds, const std::array<glm::vec4, 6>& planes);
    
        static std::vector<Bounds> GetWorldSpaceAABB(const std::vector<RenderComp*>& renderComps);
        static std::array<glm::vec4, 6> GetFrustumPlanes(const glm::mat4& vp);
        static bool FrustumCull(const Bounds& bounds, const std::array<glm::vec4, 6>& planes);
    };
}
