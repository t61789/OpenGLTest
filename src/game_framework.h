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

        uint32_t GetScreenWidth() const { return m_screenWidth;}
        uint32_t GetScreenHeight() const { return m_screenHeight;}
        
    private:
        GLFWwindow* m_window = nullptr;

        Gui* m_gui = nullptr;
        BuiltInRes* m_builtInRes = nullptr;
        RenderPipeline* m_renderPipeline = nullptr;
        GameResource* m_gameResource = nullptr;
        GlState* m_glState = nullptr;

        uint32_t m_screenWidth = 1600;
        uint32_t m_screenHeight = 900;

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
