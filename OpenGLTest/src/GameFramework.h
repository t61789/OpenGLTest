#pragma once

#include "RenderPipeline.h"
#include "Scene.h"
#include "glfw3.h"

class GameFramework
{
public:
    static GameFramework* getInstance();
    
    static float s_deltaTime;
    static float s_curFrameTime;
    static int s_frameCount;
    
    GameFramework();
    ~GameFramework();

    bool init();
    void gameLoop();

    float getDeltaTime() const;
    float getCurFrameTime() const;
    float getFrameCount() const;
    bool keyPressed(int glfwKey) const;
    
private:
    static GameFramework* s_instance;
    
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Scene> m_scene = nullptr;
    std::unique_ptr<RenderPipeline> m_renderPipeline = nullptr;

    int m_screenWidth = 1270;
    int m_screenHeight = 800;

    size_t m_setFrameBufferSizeEventHandler;

    void _initGame();
    bool _initFrame();
    bool _initGlfw();
    bool _initImGui(GLFWwindow* glfwWindow);
    void _processInput() const;
    void _frameBegin();
    void _frameEnd();
    void _beforeUpdate();
    void _update() const;
    void _render() const;
    void _onSetFrameBufferSize(GLFWwindow* window, int width, int height);
};
