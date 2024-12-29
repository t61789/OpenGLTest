#include "Gui.h"

#include <string>

#include "GameFramework.h"
#include "Utils.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void Gui::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::BeforeUpdate()
{
    ImGui::Begin("Application Info");
    ImGui::Text(std::string("FPS: " + Utils::ToString(1 / GameFramework::s_deltaTime)).c_str());
    ImGui::End();
}

void Gui::AfterUpdate()
{
    
}

void Gui::Render()
{
    ImGui::Begin("Log Output");
    if(ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true))
    {
        for (const auto& msg : Utils::s_logs) {
            ImGui::TextUnformatted(msg.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

