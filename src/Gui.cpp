#include "Gui.h"

#include <string>

#include "Utils.h"
#include "RenderPipeline.h"
#include "IndirectLighting.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

Event<> Gui::drawGuiEvent;
Event<> Gui::drawConsoleEvent;

IGui::IGui()
{
    m_drawConsoleGuiCallBack = new std::function<void()>([this]{this->OnDrawConsoleGui();});
    Gui::drawConsoleEvent.AddCallBack(m_drawConsoleGuiCallBack);
    m_drawGuiCallBack = new std::function<void()>([this]{this->OnDrawGui();});
    Gui::drawGuiEvent.AddCallBack(m_drawGuiCallBack);
}

IGui::~IGui()
{
    Gui::drawConsoleEvent.RemoveCallBack(m_drawConsoleGuiCallBack);
    delete m_drawConsoleGuiCallBack;
    Gui::drawGuiEvent.RemoveCallBack(m_drawGuiCallBack);
    delete m_drawGuiCallBack;
}

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
    
    drawGuiEvent.Invoke();
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

void Gui::DrawApplicationPanel()
{
    // ImGui::Begin("Application Info");
    // auto deltaTime = Time::GetInstance()->deltaTime;
    // ImGui::Text(std::string("FPS: " + Utils::ToString(1 / deltaTime, 2)).c_str());
    // ImGui::End();
}

void Gui::DrawLogInfoPanel()
{
    ImGui::Begin("Console");

    if (ImGui::CollapsingHeader("Application Info"))
    {
        auto deltaTime = Time::GetInstance()->deltaTime;
        ImGui::Text(std::string("FPS: " + Utils::ToString(1 / deltaTime, 2)).c_str());
    }
    
    drawGuiEvent.Invoke();
    
    if(ImGui::BeginChild("Log Info", ImVec2(0, 0), true))
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
    
    drawConsoleEvent.Invoke();
    
    // IndirectLighting::SetGradientAmbientColor(sky, equator, ground);
    
    // auto kawaseBlurIterations = ImGui::SliderInt("KawaseBlurIterations", &RenderPipeline::instance->kawaseBlurIterations, 1, 10);
    
    ImGui::End();
}
