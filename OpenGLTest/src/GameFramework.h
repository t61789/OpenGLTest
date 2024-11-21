#pragma once

#include "RenderPipeline.h"
#include "Scene.h"
#include "glfw3.h"

class GameFramework
{
public:
    static GameFramework* getInstance();
    
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

    int m_screenWidth = 800;
    int m_screenHeight = 600;

    float m_deltaTime;
    float m_curFrameTime;
    int m_frameCount;

    size_t m_setFrameBufferSizeEventHandler;

    void _initGame();
    bool _initFrame();
    bool _initGlfw();
    void _processInput() const;
    void _update() const;
    void _render() const;
    void _onSetFrameBufferSize(GLFWwindow* window, int width, int height);
};
