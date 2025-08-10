#pragma once

#include "functional"
#include "game_resource.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "render_pipeline.h"
#include "utils.h"

namespace op
{
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

        std::unique_ptr<Gui> m_gui = nullptr;
        std::unique_ptr<BuiltInRes> m_builtInRes = nullptr;
        std::unique_ptr<RenderPipeline> m_renderPipeline = nullptr;
        std::unique_ptr<GameResource> m_gameResource = nullptr;

        int m_screenWidth = 1600;
        int m_screenHeight = 900;

        EventHandler m_setFrameBufferSizeHandler = 0;

        Scene* m_scene = nullptr;

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
