#pragma once
#include "time_out_buffer.h"

namespace op
{
    class Object;

    class ControlPanelUi : public Singleton<ControlPanelUi>
    {
    public:
        class UiProxy
        {
        public:
            explicit UiProxy();
            virtual ~UiProxy();

            virtual void DrawConsoleUi() = 0;

        private:
            EventHandler m_drawConsoleUiHandler = 0;
        };
    
        ~ControlPanelUi();
    
        void Draw();

    private:
        TimeOutBuffer<std::string, bool> m_foldout = TimeOutBuffer<std::string, bool>(true);

        static float s_intent;
        Object* m_selected = nullptr;

        Event<> m_drawConsoleUiEvent;

        void DrawSceneInfo();
        void DrawHierarchy(Object* obj);
        void DrawProperties(const Object* obj);
        void DrawApplicationInfo();
        void DrawLogInfo();
    };
}