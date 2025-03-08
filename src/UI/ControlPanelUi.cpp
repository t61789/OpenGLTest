#include "ControlPanelUi.h"

#include "imgui.h"

#include "Object.h"
#include "Scene.h"
#include "GameFramework.h"

float ControlPanelUi::s_intent = 20.0f;

ControlPanelUi::~ControlPanelUi()
{
    if (m_selected)
    {
        DECREF(m_selected);
    }
}

void ControlPanelUi::OnDrawGui()
{
    auto scene = GameFramework::GetInstance()->scene;
    if (!scene)
    {
        return;
    }
    
    ImGui::BeginChild("Hierarchy", ImVec2(0, 150), true);
    DrawHierarchy(scene->sceneRoot);
    ImGui::EndChild();
        
    ImGui::BeginChild("Properties", ImVec2(0, 150), true);
    if (m_selected)
    {
        DrawProperties(m_selected);
    }
    else
    {
        ImGui::Text("未选择任何物体");
    }
    ImGui::EndChild();
}

void ControlPanelUi::DrawHierarchy(Object* obj)
{
    ImGui::TextUnformatted(obj->name.c_str());
    ImGui::SameLine();
    auto pathInScene = obj->GetPathInScene();
    if (ImGui::Button(("pick##" + pathInScene).c_str()))
    {
        if (m_selected != obj)
        {
            if (m_selected)
            {
                DECREF(m_selected);
            }

            m_selected = obj;
            INCREF(m_selected);
        }
    }

    if (!obj->children.empty())
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
            for (auto child : obj->children)
            {
                DrawHierarchy(child);
            }
            ImGui::Unindent(s_intent);
        }
    }
}

void ControlPanelUi::DrawProperties(Object* obj)
{
    obj->position = Gui::DragFloat3("position", obj->position, 0.02f);
    obj->scale = Gui::DragFloat3("scale", obj->scale, 0.02f);
    obj->rotation = Gui::DragFloat3("rotation", obj->rotation, 1.0f);
}
