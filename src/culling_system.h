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

        bool CullOnce(const Bounds& bounds, const std::array<Vec4, 6>& planes);
    };
}
