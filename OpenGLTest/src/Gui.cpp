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
    DrawApplicationPanel();
}

void Gui::OnGui()
{
    DrawCoordinateDirLine();

    DrawConsolePanel();
}

void Gui::AfterUpdate()
{
    
}

void Gui::Render()
{
    DrawLogInfoPanel();
    
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
    RenderPipeline::instance->getViewProjMatrix(view, proj);
    RenderPipeline::instance->getScreenSize(width, height);
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

void Gui::DrawApplicationPanel()
{
    ImGui::Begin("Application Info");
    ImGui::Text(std::string("FPS: " + Utils::ToString(1 / GameFramework::s_deltaTime, 2)).c_str());
    ImGui::End();
}

void Gui::DrawLogInfoPanel()
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
}

void Gui::DrawConsolePanel()
{
    ImGui::Begin("Console");

    auto sky = SliderFloat3("GradientSky", IndirectLighting::s_gradientAmbientColor.sky, 0.0f, 1.0f, "%.2f");
    auto equator = SliderFloat3("GradientEquator", IndirectLighting::s_gradientAmbientColor.equator, 0.0f, 1.0f, "%.2f");
    auto ground = SliderFloat3("GradientGround", IndirectLighting::s_gradientAmbientColor.ground, 0.0f, 1.0f, "%.2f");
    IndirectLighting::SetGradientAmbientColor(sky, equator, ground);
    
    
    ImGui::End();
}
