#pragma once

#include "functional"

#include "RenderPipeline.h"

class ControlPanelUi;
class Scene;

class GameFramework
{
public:
    static GameFramework* GetInstance();

    Scene* scene = nullptr;
    
    GameFramework();
    ~GameFramework();

    bool Init();
    void GameLoop();

    bool KeyPressed(int glfwKey) const;
    
private:
    static GameFramework* s_instance;
    
    GLFWwindow* m_window = nullptr;
    
    std::unique_ptr<RenderPipeline> m_renderPipeline = nullptr;
    std::unique_ptr<ControlPanelUi> m_controlPanelUi = nullptr;

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
