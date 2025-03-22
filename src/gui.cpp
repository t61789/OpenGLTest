#include "gui.h"

#include <string>

#include "utils.h"
#include "render_pipeline.h"
#include "ui/control_panel_ui.h"
#include "backends\imgui_impl_glfw.h"
#include "backends\imgui_impl_opengl3.h"

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

    void Gui::Render()
    {
        DrawCoordinateDirLine();
        
        DrawConsolePanel();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Gui::DrawCoordinateDirLine()
    {
        if(RenderPipeline::instance == nullptr)
        {
            return;
        }
        
        glm::mat4 view, proj;
        int width, height;
        RenderPipeline::instance->GetViewProjMatrix(view, proj);
        RenderPipeline::instance->GetScreenSize(width, height);
        auto screenSize = glm::vec2(static_cast<float>(width), static_cast<float>(height));
        
        Utils::DebugDrawLine(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), view, proj, screenSize, IM_COL32(255, 0, 0, 255));
        Utils::DebugDrawLine(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), view, proj, screenSize, IM_COL32(0, 255, 0, 255));
        Utils::DebugDrawLine(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), view, proj, screenSize, IM_COL32(0, 0, 255, 255));
    }

    glm::vec3 Gui::SliderFloat3(const std::string& label, const glm::vec3 input, const float v_min, const float v_max, const std::string& format)
    {
        auto arr = Utils::ToArr(input);
        ImGui::SliderFloat3(label.c_str(), arr, v_min, v_max, format.c_str());
        auto result = Utils::FromArr(arr);
        delete[] arr;
        return result;
    }

    glm::vec3 Gui::InputFloat3(const std::string& label, glm::vec3 input, const std::string& format)
    {
        auto arr = Utils::ToArr(input);
        ImGui::InputFloat3(label.c_str(), arr, format.c_str());
        auto result = Utils::FromArr(arr);
        delete[] arr;
        return result;
    }

    glm::vec3 Gui::DragFloat3(const std::string& label, glm::vec3 input, float speed, const std::string& format)
    {
        auto arr = Utils::ToArr(input);
        ImGui::DragFloat3(label.c_str(), arr, speed, 0, 0, format.c_str());
        auto result = Utils::FromArr(arr);
        delete[] arr;
        return result;
    }

    void Gui::DrawConsolePanel()
    {
        ImGui::Begin("Console");

        m_controlPanelUi->Draw();
        
        ImGui::End();
    }
}
