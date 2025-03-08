#pragma once
#include "Gui.h"
#include "TimeOutBuffer.h"

class Object;

class ControlPanelUi : public IGui
{
public:
    ~ControlPanelUi();
    
    void OnDrawGui() override;

private:
    TimeOutBuffer<std::string, bool> m_foldout = TimeOutBuffer<std::string, bool>(true);

    static float s_intent;
    Object* m_selected = nullptr;
    
    void DrawHierarchy(Object* obj);
    void DrawProperties(Object* obj);
};
