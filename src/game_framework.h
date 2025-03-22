#pragma once

#include "functional"

#include "glfw3.h"
#include "RenderPipeline.h"
#include "Utils.h"

namespace op
{
    class BuiltInRes;
    class Scene;
    class Gui;

    class GameFramework : public Singleton<GameFramework>
    {
    public:
        Scene* scene = nullptr;
        
        GameFramework();
        ~GameFramework();

        bool Init();
        void GameLoop();

        bool KeyPressed(int glfwKey) const;
        
    private:
        GLFWwindow* m_window = nullptr;

        std::unique_ptr<Time> m_time = nullptr;
        std::unique_ptr<Gui> m_gui = nullptr;
        std::unique_ptr<BuiltInRes> m_builtInRes = nullptr;
        std::unique_ptr<RenderPipeline> m_renderPipeline = nullptr;

        int m_screenWidth = 1270;
        int m_screenHeight = 800;

        std::function<void(GLFWwindow*, int, int)>* m_setFrameBufferSizeCallBack = nullptr;

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
        void OnSetFrameBufferSize(GLFWwindow* window, int width, int height);
    };
}
