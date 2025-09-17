#include "control_panel_ui.h"

#include "imgui.h"

#include "gui.h"
#include "object.h"
#include "scene.h"
#include "game_framework.h"
#include "game_resource.h"
#include "objects/transform_comp.h"

namespace op
{
    float ControlPanelUi::s_intent = 20.0f;

    ControlPanelUi::UiProxy::UiProxy()
    {
        m_drawConsoleUiHandler = Ins()->m_drawConsoleUiEvent.Add(this, &UiProxy::DrawConsoleUi);
    }

    ControlPanelUi::UiProxy::~UiProxy()
    {
        Ins()->m_drawConsoleUiEvent.Remove(m_drawConsoleUiHandler);
    }

    void ControlPanelUi::Draw()
    {
        DrawApplicationInfo();

        DrawSceneInfo();
    
        DrawLogInfo();

        m_drawConsoleUiEvent.Invoke();
    }

    void ControlPanelUi::DrawSceneInfo()
    {
        auto scene = GameFramework::Ins()->GetMainScene();
        if (!scene)
        {
            return;
        }

        if (!ImGui::CollapsingHeader("Scene Info"))
        {
            return;
        }
    
        ImGui::BeginChild("Hierarchy", ImVec2(0, 150), true);
        DrawHierarchy(scene->GetRoot());
        ImGui::EndChild();
        
        DrawProperties(m_selected.lock().get());
    }

    void ControlPanelUi::DrawHierarchy(crsp<Object> obj)
    {
        ImGui::TextUnformatted(obj->name.CStr());
        ImGui::SameLine();
        auto pathInScene = obj->GetPathInScene();
        if (ImGui::Button(("pick##" + pathInScene).c_str()))
        {
            if (m_selected.expired() || m_selected.lock() != obj)
            {
                m_selected = obj;
            }
        }

        if (!obj->GetChildren().empty())
        {
            auto foldout = m_foldout.GetOrAdd(pathInScene);
            ImGui::SameLine();
            auto foldoutLabel = foldout ? "-##" + pathInScene : "+##" + pathInScene;
            if (ImGui::Button(foldoutLabel.c_str()))
            {
                foldout = !foldout;
                m_foldout.Set(pathInScene, foldout);
            }

            if (foldout)
            {
                ImGui::Indent(s_intent);
                for (auto& child : obj->GetChildren())
                {
                    DrawHierarchy(child);
                }
                ImGui::Unindent(s_intent);
            }
        }
    }

    void ControlPanelUi::DrawProperties(const Object* obj)
    {
        ImGui::BeginChild("Properties", ImVec2(0, 150), true);
        
        if (obj)
        {
            ImGui::TextUnformatted(obj->name.CStr());
            obj->transform->SetPosition(Gui::DragFloat3("position", obj->transform->GetPosition(), 0.02f));
            obj->transform->SetScale(Gui::DragFloat3("scale", obj->transform->GetScale(), 0.02f));
            obj->transform->SetEulerAngles(Gui::DragFloat3("rotation", obj->transform->GetEulerAngles(), 1.0f));
        }
        else
        {
            ImGui::Text("未选择任何物体");
        }
    
        ImGui::EndChild();
    }

    void ControlPanelUi::DrawApplicationInfo()
    {
        if (ImGui::CollapsingHeader("Application Info"))
        {
            auto deltaTime = GetGR()->time.deltaTime;
            ImGui::Text(std::string("FPS: " + to_string(1 / deltaTime, 2)).c_str());
        }
    }

    void ControlPanelUi::DrawLogInfo()
    {
        if (ImGui::CollapsingHeader("Log Info"))
        {
            if (ImGui::BeginChild("Log Info", ImVec2(0, 0), true))
            {
                for (const auto& msg : Utils::s_logs) {
                    ImGui::TextUnformatted(msg.c_str());
                }
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                    ImGui::SetScrollHereY(1.0f);
                }
            }
            ImGui::EndChild();
        }
    }
}
