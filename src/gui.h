#pragma once
#include <string>

#include "utils.h"
#include "glm/glm.hpp"
#include "ui/control_panel_ui.h"

namespace op
{
    class Gui : public Singleton<Gui>
    {
    public:
        Gui();
        
        void BeginFrame();
        void BeforeUpdate();
        void AfterUpdate();
        void Render();

        static glm::vec3 SliderFloat3(const std::string& label, glm::vec3 input, float v_min, float v_max, const std::string& format = "%.2f");
        static glm::vec3 InputFloat3(const std::string& label, glm::vec3 input, const std::string& format = "%.2f");
        static glm::vec3 DragFloat3(const std::string& label, glm::vec3 input, float speed = 1, const std::string& format = "%.2f");

    private:
        std::unique_ptr<ControlPanelUi> m_controlPanelUi = nullptr;
            
        void DrawConsolePanel();
        
        static void DrawCoordinateDirLine();
    };
}
