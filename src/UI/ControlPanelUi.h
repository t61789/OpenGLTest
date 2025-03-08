#pragma once
#include "Gui.h"
#include "TimeOutBuffer.h"

class Object;

class ControlPanelUi : public IGui
{
public:
    void OnDrawGui() override;

private:
    TimeOutBuffer<std::string, bool> m_foldout = TimeOutBuffer<std::string, bool>(true);

    static float s_intent;
    
    void DrawObj(Object* obj, Object*& selected);
};
