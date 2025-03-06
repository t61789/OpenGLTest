#pragma once

#include "RenderPipeline.h"
#include "Scene.h"
#include "glfw3.h"

class GameFramework
{
public:
    static GameFramework* GetInstance();
    
    static float s_deltaTime;
    static float s_curFrameTime;
    static int s_frameCount;
    
    GameFramework();
    ~GameFramework();

    bool Init();
    void GameLoop();

    float GetDeltaTime() const;
    float GetCurFrameTime() const;
    int GetFrameCount() const;
    bool KeyPressed(int glfwKey) const;
    
private:
    static GameFramework* s_instance;
    
    GLFWwindow* m_window = nullptr;
    Scene* m_scene = nullptr;
    RenderPipeline* m_renderPipeline = nullptr;

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
