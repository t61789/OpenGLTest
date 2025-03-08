#pragma once
#include <functional>
#include <string>

#include "Event.h"
#include "glm/glm.hpp"

class IGui
{
public:
    IGui();
    virtual ~IGui();
    virtual void OnDrawGui(){}
    virtual void OnDrawConsoleGui(){}

private:
    std::function<void()>* m_drawGuiCallBack = nullptr;
    std::function<void()>* m_drawConsoleGuiCallBack = nullptr;   
};

class Gui
{
public:
    static void BeginFrame();
    static void BeforeUpdate();
    static void OnGui();
    static void AfterUpdate();
    static void Render();

    static Event<> drawGuiEvent;
    static Event<> drawConsoleEvent;

private:
    static void DrawApplicationPanel();
    static void DrawLogInfoPanel();
    static void DrawConsolePanel();
    static void DrawCoordinateDirLine();

    static glm::vec3 SliderFloat3(const std::string& label, glm::vec3 input, float v_min, float v_max, const std::string& format);
};
