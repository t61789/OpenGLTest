﻿#include "GameFramework.h"

#include <iostream>

#include "Camera.h"
#include "Gui.h"
#include "imgui.h"
#include "Windows.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glad/glad.h"

GameFramework* GameFramework::s_instance = nullptr;
float GameFramework::s_deltaTime = 0;
float GameFramework::s_curFrameTime = 0;
int GameFramework::s_frameCount = -1;

bool initGl()
{
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR>> Failed to initialize GLAD\n";
        return false;
    }

    auto version = (const char *)glGetString(GL_VERSION);
    printf("OpenGL Version: %s\n", version);

    int maxVertexAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttributes);
    std::cout << "Max vertex attributes: " << maxVertexAttributes << "\n";

    glEnable(GL_DEPTH_TEST);

    return true;
}

GameFramework* GameFramework::getInstance()
{
    return s_instance;
}

GameFramework::GameFramework()
{
    if(s_instance != nullptr)
    {
        throw std::runtime_error("GameFramework instance already exists");
    }
    
    s_instance = this;
    
    m_setFrameBufferSizeEventHandler = Utils::s_setFrameBufferSizeEvent.addListener(
        [this](GLFWwindow* window, const int width, const int height)
        {
            this->_onSetFrameBufferSize(window, width, height);
        });
}

GameFramework::~GameFramework()
{
    Utils::s_setFrameBufferSizeEvent.removeListener(m_setFrameBufferSizeEventHandler);

    s_instance = nullptr;
}

bool GameFramework::init()
{
    if (!_initFrame())
    {
        return false;
    }

    _initGame();

    return true;
}

void GameFramework::gameLoop()
{
    // Render loop
    s_frameCount = -1;
    while (!glfwWindowShouldClose(m_window))
    {
        _frameBegin();

        _processInput();

        _beforeUpdate();

        _update();

        _render();
        
        _frameEnd();
    }

    if(m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

float GameFramework::getDeltaTime() const
{
    return s_deltaTime;
}

float GameFramework::getCurFrameTime() const
{
    return s_curFrameTime;
}

float GameFramework::getFrameCount() const
{
    return static_cast<float>(s_frameCount);
}

bool GameFramework::keyPressed(const int glfwKey) const
{
    return glfwGetKey(m_window, glfwKey) == GLFW_PRESS;
}

bool GameFramework::_initFrame()
{
    if(!_initGlfw())
    {
        return false;
    }

    if(!initGl())
    {
        return false;
    }

    if(!_initImGui(m_window))
    {
        return false;
    }

    return true;
}


bool GameFramework::_initGlfw()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    m_window = glfwCreateWindow(m_screenWidth, m_screenHeight, "YeahTitle", nullptr, nullptr);
    if(!m_window)
    {
        std::cout << "Failed to create a window\n";
        glfwTerminate();

        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(
        m_window,
        [](GLFWwindow* window, const int width, const int height)
        {
            Utils::s_setFrameBufferSizeEvent.invoke(window, width, height);
        });

    return true;
}

bool GameFramework::_initImGui(GLFWwindow* glfwWindow)
{
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    ImGuiIO& io = ImGui::GetIO();
    // 加载中文字体（确保你的项目中有相应的字体文件）
    io.Fonts->AddFontFromFileTTF(Utils::GetRealAssetPath("msyh.ttc").c_str(), 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    // 重新创建字体纹理
    ImGui_ImplOpenGL3_CreateFontsTexture();
    
    return true;
}

void GameFramework::_processInput() const
{
    if(keyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(m_window, true);
    }
}

void GameFramework::_frameBegin()
{
    Gui::BeginFrame();
    
    s_frameCount++;
    auto curFrameTime = static_cast<float>(glfwGetTime());
    s_deltaTime = curFrameTime - s_curFrameTime;
    s_curFrameTime = curFrameTime;
}

void GameFramework::_frameEnd()
{
    
}

void UpdateObject(const OBJECT_ID objId)
{
    auto obj = ResourceMgr::GetPtr<Object>(objId);
    if(obj != nullptr)
    {
        obj->update();
        for (auto child : obj->children)
        {
            UpdateObject(child);
        }
    }
}

void GameFramework::_beforeUpdate()
{
    Gui::BeforeUpdate();

    Gui::OnGui();
}

void GameFramework::_update() const
{
    if(m_scene != nullptr)
    {
        UpdateObject(m_scene->sceneRoot);
    }
    
    // _entity->rotation.y += GetDeltaTime() * 60.0f;
}

void GameFramework::_render() const
{
    auto mainCamera = Camera::GetMainCamera();
    if(mainCamera != UNDEFINED_RESOURCE)
    {
        m_renderPipeline->render(mainCamera, m_scene.get());
    }
    else
    {
        std::cout << "未找到可用摄像机\n";
        Sleep(16);
    }
}

void GameFramework::_onSetFrameBufferSize(GLFWwindow* window, const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    glViewport(0, 0, width, height);
    m_renderPipeline->setScreenSize(width, height);
}

void GameFramework::_initGame()
{
    m_renderPipeline = std::make_unique<RenderPipeline>(m_screenWidth, m_screenHeight, m_window);
    m_scene = std::make_unique<Scene>("TestScene.json");
}

