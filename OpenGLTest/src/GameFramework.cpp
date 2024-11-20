#include "GameFramework.h"

#include <iostream>
#include "Windows.h"

bool initGl()
{
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR>> Failed to initialize GLAD\n";
        return false;
    }

    int maxVertexAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttributes);
    std::cout << "Max vertex attributes: " << maxVertexAttributes << "\n";

    glEnable(GL_DEPTH_TEST);

    return true;
}

GameFramework* GameFramework::instance = nullptr;

GameFramework::GameFramework()
{
    instance = this;
}

GameFramework::~GameFramework()
{
    delete m_renderPipeline;
    delete m_scene;

    instance = nullptr;
}

bool GameFramework::Init()
{
    if (!InitFrame())
    {
        return false;
    }

    InitGame();

    return true;
}

void GameFramework::StartGameLoop()
{
    // Render loop
    double timeCount = 0;
    double preFrameTime = 0;
    int frameCount = 0;
    _frameCount = -1;
    while (!glfwWindowShouldClose(_window))
    {
        _frameCount++;
        frameCount++;
        _curFrameTime = glfwGetTime();
        _deltaTime = static_cast<float>(_curFrameTime - preFrameTime);
        if (_curFrameTime - timeCount > 1)
        {
            std::cout << "Frame Rate: " << frameCount / (_curFrameTime - timeCount) << "\n";
            frameCount = 0;
            timeCount = _curFrameTime;
        }

        ProcessInput();

        Update();

        // Render
        Render();
        
        preFrameTime = _curFrameTime;
    }

    glfwTerminate();
}

float GameFramework::GetDeltaTime()
{
    return _deltaTime;
}

float GameFramework::GetCurFrameTime()
{
    return _curFrameTime;
}

float GameFramework::GetFrameCount()
{
    return _frameCount;
}

bool GameFramework::KeyPressed(int glfwKey) const
{
    return glfwGetKey(_window, glfwKey) == GLFW_PRESS;
}

bool GameFramework::InitFrame()
{
    if(!InitGlfw())
    {
        return false;
    }

    if(!initGl())
    {
        return false;
    }

    return true;
}


bool GameFramework::InitGlfw()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _window = glfwCreateWindow(_screenWidth, _screenHeight, "YeahTitle", nullptr, nullptr);
    if(!_window)
    {
        std::cout << "Failed to create a window\n";
        glfwTerminate();

        return false;
    }
    glfwMakeContextCurrent(_window);
    glfwSetFramebufferSizeCallback(_window, FRAME_BUFFER_SIZE_CALL_BACK);

    return true;
}

void GameFramework::ProcessInput()
{
    if(KeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(_window, true);
    }
}

void UpdateObject(OBJECT_ID objId)
{
    auto obj = ResourceMgr::GetPtr<Object>(objId);
    if(obj != nullptr)
    {
        obj->Update();
        for (auto child : obj->m_children)
        {
            UpdateObject(child);
        }
    }
}

void GameFramework::Update()
{
    if(m_scene != nullptr)
    {
        UpdateObject(m_scene->m_sceneRoot);
    }
    
    // _entity->rotation.y += GetDeltaTime() * 60.0f;
}

void GameFramework::Render()
{
    auto mainCamera = Camera::GetMainCamera();
    if(mainCamera != UNDEFINED_RESOURCE)
    {
        m_renderPipeline->Render(mainCamera, m_scene->m_sceneRoot);
    }
    else
    {
        std::cout << "未找到可用摄像机\n";
        Sleep(16);
    }
}

void GameFramework::FRAME_BUFFER_SIZE_CALL_BACK(GLFWwindow* window, int width, int height)
{
    std::cout << width << " " << height << "\n";
    instance->_screenWidth = width;
    instance->_screenHeight = height;
    glViewport(0, 0, width, height);
    instance->m_renderPipeline->SetScreenSize(width, height);
}

void GameFramework::InitGame()
{
    m_renderPipeline = new RenderPipeline(_screenWidth, _screenHeight, _window);

    m_scene = new Scene("TestScene.json");
}

