#pragma once

#include "utils.h"

namespace op
{
    class RenderPipeline;
    class GlState;
    class GameResource;
    class BuiltInRes;
    class Scene;
    class Gui;

    class GameFramework : public Singleton<GameFramework>
    {
    public:
        GameFramework();
        ~GameFramework();

        bool Init();
        void GameLoop();

        bool KeyPressed(int glfwKey) const;

        Scene* GetMainScene() const;
        
    private:
        GLFWwindow* m_window = nullptr;

        Gui* m_gui = nullptr;
        BuiltInRes* m_builtInRes = nullptr;
        RenderPipeline* m_renderPipeline = nullptr;
        GameResource* m_gameResource = nullptr;

        int m_screenWidth = 1600;
        int m_screenHeight = 900;

        EventHandler m_onFrameBufferResizeHandler = 0;

        sp<Scene> m_scene = nullptr;

        void InitGame();
        void ReleaseGame();
        bool InitFrame();
        bool InitGlfw();
        bool InitImGui(GLFWwindow* glfwWindow);
        void ProcessInput() const;
        void FrameBegin();
        void FrameEnd();
        void BeforeUpdate();
        void Update();
        void Render() const;
        void OnSetFrameBufferResize(GLFWwindow* window, int width, int height);
    };
}
