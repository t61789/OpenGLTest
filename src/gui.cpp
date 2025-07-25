#include "gui.h"

#include <string>

#include "utils.h"
#include "render_pipeline.h"
#include "ui/control_panel_ui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace op
{
    Gui::Gui()
    {
        m_controlPanelUi = std::make_unique<ControlPanelUi>();
    }

    void Gui::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Gui::BeforeUpdate()
    {
        
    }

    void Gui::AfterUpdate()
    {
    }

    void Gui::Render(const RenderContext* renderContext)
    {
        DrawCoordinateDirLine(renderContext);

        // DrawBounds(renderContext);
        
        DrawConsolePanel();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Gui::DrawCoordinateDirLine(const RenderContext* renderContext)
    {
        if (!renderContext)
        {
            return;
        }
        
        const auto& view = renderContext->vMatrix;
        const auto& proj = renderContext->pMatrix;
        auto width = renderContext->screenWidth;
        auto height = renderContext->screenHeight;
        
        RenderPipeline::instance->GetScreenSize(width, height);
        auto screenSize = Vec2(static_cast<float>(width), static_cast<float>(height));
        
        Utils::DebugDrawLine(Vec3::Zero(), Vec3::Right(), view, proj, screenSize, IM_COL32(255, 0, 0, 255));
        Utils::DebugDrawLine(Vec3::Zero(), Vec3::Up(), view, proj, screenSize, IM_COL32(0, 255, 0, 255));
        Utils::DebugDrawLine(Vec3::Zero(), Vec3::Forward(), view, proj, screenSize, IM_COL32(0, 0, 255, 255));
    }

    void Gui::DrawBounds(const RenderContext* renderContext)
    {
        if (!renderContext)
        {
            return;
        }
        
        const auto& view = renderContext->vMatrix;
        const auto& proj = renderContext->pMatrix;
        auto width = renderContext->screenWidth;
        auto height = renderContext->screenHeight;
        
        auto screenSize = Vec2(static_cast<float>(width), static_cast<float>(height));
        for (auto const& renderComp : renderContext->visibleRenderObjs)
        {
            Utils::DebugDrawCube(
                renderComp->GetWorldBounds(),
                view,
                proj,
                screenSize);
        }
    }

    Vec3 Gui::SliderFloat3(const std::string& label, const Vec3 input, const float v_min, const float v_max, const std::string& format)
    {
        auto tmp = input;
        ImGui::SliderFloat3(label.c_str(), &tmp.x, v_min, v_max, format.c_str());
        return tmp;
    }

    Vec3 Gui::InputFloat3(const std::string& label, const Vec3 input, const std::string& format)
    {
        auto tmp = input;
        ImGui::InputFloat3(label.c_str(), &tmp.x, format.c_str());
        return tmp;
    }

    Vec3 Gui::DragFloat3(const std::string& label, const Vec3 input, float speed, const std::string& format)
    {
        auto tmp = input;
        ImGui::DragFloat3(label.c_str(), &tmp.x, speed, 0, 0, format.c_str());
        return tmp;
    }

    void Gui::DrawConsolePanel()
    {
        ImGui::Begin("Console");

        m_controlPanelUi->Draw();
        
        ImGui::End();
    }
}
