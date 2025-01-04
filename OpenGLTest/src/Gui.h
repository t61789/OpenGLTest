#pragma once
#include "glm.hpp"

#include "imgui.h"

class Gui
{
public:
    static void BeginFrame();
    static void BeforeUpdate();
    static void OnGui();
    static void AfterUpdate();
    static void Render();

private:
    static void DrawApplicationPanel();
    static void DrawLogInfoPanel();
    static void DrawConsolePanel();
    static void DrawCoordinateDirLine();
};
