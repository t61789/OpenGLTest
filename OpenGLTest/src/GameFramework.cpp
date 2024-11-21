#include "GameFramework.h"

#include <iostream>

#include "Camera.h"
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
    m_frameCount = -1;
    while (!glfwWindowShouldClose(m_window))
    {
        m_frameCount++;
        frameCount++;
        m_curFrameTime = glfwGetTime();
        m_deltaTime = static_cast<float>(m_curFrameTime - preFrameTime);
        if (m_curFrameTime - timeCount > 1)
        {
            std::cout << "Frame Rate: " << frameCount / (m_curFrameTime - timeCount) << "\n";
            frameCount = 0;
            timeCount = m_curFrameTime;
        }

        ProcessInput();

        Update();

        // Render
        Render();
        
        preFrameTime = m_curFrameTime;
    }

    glfwTerminate();
}

float GameFramework::GetDeltaTime()
{
    return m_deltaTime;
}

float GameFramework::GetCurFrameTime()
{
    return m_curFrameTime;
}

float GameFramework::GetFrameCount()
{
    return m_frameCount;
}

bool GameFramework::KeyPressed(int glfwKey) const
{
    return glfwGetKey(m_window, glfwKey) == GLFW_PRESS;
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

    m_window = glfwCreateWindow(m_screenWidth, m_screenHeight, "YeahTitle", nullptr, nullptr);
    if(!m_window)
    {
        std::cout << "Failed to create a window\n";
        glfwTerminate();

        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, FRAME_BUFFER_SIZE_CALL_BACK);

    return true;
}

void GameFramework::ProcessInput()
{
    if(KeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(m_window, true);
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
        m_renderPipeline->Render(mainCamera, m_scene);
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
    instance->m_screenWidth = width;
    instance->m_screenHeight = height;
    glViewport(0, 0, width, height);
    instance->m_renderPipeline->SetScreenSize(width, height);
}

void GameFramework::InitGame()
{
    m_renderPipeline = new RenderPipeline(m_screenWidth, m_screenHeight, m_window);

    m_scene = new Scene("TestScene.json");
}

