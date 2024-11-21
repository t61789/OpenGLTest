#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glfw3.h>

#include "RenderPipeline.h"
#include "Scene.h"

class GameFramework
{
public:
    static GameFramework* instance;

    GameFramework();
    ~GameFramework();

    bool Init();
    void GameLoop();

    float GetDeltaTime() const;
    float GetCurFrameTime() const;
    float GetFrameCount() const;
    bool KeyPressed(int glfwKey) const;
    
private:
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Scene> m_scene = nullptr;
    std::unique_ptr<RenderPipeline> m_renderPipeline = nullptr;

    int m_screenWidth = 800;
    int m_screenHeight = 600;

    float m_deltaTime;
    float m_curFrameTime;
    int m_frameCount;

    void InitGame();
    bool InitFrame();
    bool InitGlfw();
    void ProcessInput() const;
    void Update() const;
    void Render() const;
    
    static void FRAME_BUFFER_SIZE_CALL_BACK(GLFWwindow* window, int width, int height);
};
