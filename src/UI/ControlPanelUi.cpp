#include "ControlPanelUi.h"

#include "imgui.h"

#include "Object.h"
#include "Scene.h"
#include "GameFramework.h"

float ControlPanelUi::s_intent = 20.0f;

void ControlPanelUi::OnDrawGui()
{
    auto scene = GameFramework::GetInstance()->scene;
    if (!scene)
    {
        return;
    }
    
    ImGui::Begin("Hierarchy");

    auto intent = 0;
    Object* selected = nullptr;
    DrawObj(scene->sceneRoot, selected);

    ImGui::End();
}

void ControlPanelUi::DrawObj(Object* obj, Object*& selected)
{
    ImGui::TextUnformatted(obj->name.c_str());
    ImGui::SameLine();
    auto pathInScene = obj->GetPathInScene();
    if (ImGui::Button(("pick##" + pathInScene).c_str()))
    {
        selected = obj;
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
                DrawObj(child, selected);
            }
            ImGui::Unindent(s_intent);
        }
    }
}
