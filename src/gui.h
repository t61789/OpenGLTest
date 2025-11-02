#pragma once
#include <string>

#include "render_context.h"
#include "utils.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class Gui : public Singleton<Gui>
    {
        struct DrawLineCmd
        {
            Vec3 start;
            Vec3 end;
            ImU32 color;
            float thickness;
        };
        
    public:
        Gui();
        
        void BeginFrame();
        void BeforeUpdate();
        void AfterUpdate();
        void Render(const RenderContext* renderContext);
        
        void DrawLine(cr<Vec3> start, cr<Vec3> end, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);
        void DrawFrustumPlanes(cr<Matrix4x4> vpMatrix, ImU32 color = IM_COL32(0, 0, 255, 255), float thickness = 1.0f);
        void DrawFrustumPlanes(const Vec3* corners, ImU32 color = IM_COL32(0, 0, 255, 255), float thickness = 1.0f);

        static Vec3 SliderFloat3(const std::string& label, Vec3 input, float v_min, float v_max, const std::string& format = "%.2f");
        static Vec3 InputFloat3(const std::string& label, Vec3 input, const std::string& format = "%.2f");
        static Vec3 DragFloat3(const std::string& label, Vec3 input, float speed = 1, const std::string& format = "%.2f");

    private:
        std::unique_ptr<ControlPanelUi> m_controlPanelUi = nullptr;
        vec<DrawLineCmd> m_drawLineCmds;
            
        void DrawBounds(const RenderContext* renderContext);
        void DrawCoordinateDirLine(const RenderContext* renderContext);
        void DrawCube(const Bounds& bounds, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);
        
        void DrawConsolePanel();
        void DoDrawLines();
        
        static void ImGuiDrawLine(const Vec3& worldStart, const Vec3& worldEnd, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, const Vec2& screenSize, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);
    };
}
