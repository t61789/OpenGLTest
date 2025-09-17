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

    class CullingSystem final : public ControlPanelUi::UiProxy
    {
    public:
        void Cull();

        void DrawConsoleUi() override;

    private:
        vec<Bounds> m_bounds;

        bool CullOnce(const Bounds& bounds, const std::array<Vec4, 6>& planes);
    };
}
