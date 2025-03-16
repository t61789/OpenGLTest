#include "GameFramework.h"

#include <iostream>

#include "glad.h"
#include "glfw3.h"
#include "imgui.h"

#include "Utils.h"
#include "Scene.h"
#include "Gui.h"
#include "Windows.h"
#include "Objects/CameraComp.h"
#include "Ui/ControlPanelUi.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

GameFramework* GameFramework::s_instance = nullptr;

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
    
    // glEnable(GL_FRAMEBUFFER_SRGB);

    return true;
}

GameFramework* GameFramework::GetInstance()
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
    
    m_setFrameBufferSizeCallBack = new std::function<void(GLFWwindow*, int, int)>
        ([this](GLFWwindow* window, const int width, const int height)
        {
            this->OnSetFrameBufferSize(window, width, height);
        });
    Utils::s_setFrameBufferSizeEvent.AddCallBack(m_setFrameBufferSizeCallBack);
}

GameFramework::~GameFramework()
{
    Utils::s_setFrameBufferSizeEvent.RemoveCallBack(m_setFrameBufferSizeCallBack);
    delete m_setFrameBufferSizeCallBack;

    ReleaseGame();

    s_instance = nullptr;
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

void GameFramework::GameLoop()
{
    // Render loop
    while (!glfwWindowShouldClose(m_window))
    {
        FrameBegin();

        ProcessInput();

        BeforeUpdate();

        Update();

        Render();
        
        FrameEnd();
    }

    if(m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool GameFramework::KeyPressed(const int glfwKey) const
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

    if(!InitImGui(m_window))
    {
        return false;
    }

    return true;
}


bool GameFramework::InitGlfw()
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
            Utils::s_setFrameBufferSizeEvent.Invoke(window, width, height);
        });

    return true;
}

bool GameFramework::InitImGui(GLFWwindow* glfwWindow)
{
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    // 加载中文字体（确保你的项目中有相应的字体文件）
    io.Fonts->AddFontFromFileTTF(Utils::GetAbsolutePath("others/msyh.ttc").c_str(), 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    // 重新创建字体纹理
    ImGui_ImplOpenGL3_CreateFontsTexture();
    
    return true;
}

void GameFramework::ProcessInput() const
{
    if(KeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(m_window, true);
    }
}

void GameFramework::FrameBegin()
{
    Gui::BeginFrame();

    auto time = Time::GetInstance();
    time->frame += 1;
    auto curFrameTime = static_cast<float>(glfwGetTime());
    time->deltaTime = max(curFrameTime - time->time, 0.000001f);
    time->time = curFrameTime;
}

void GameFramework::FrameEnd()
{
    
}

void UpdateObject(Object* obj)
{
    if(obj != nullptr)
    {
        auto comps = obj->GetComps();
        for (auto comp : comps)
        {
            comp->Update();
        }
        
        for (auto child : obj->children)
        {
            UpdateObject(child);
        }
    }
}

void GameFramework::BeforeUpdate()
{
    Gui::BeforeUpdate();

    Gui::OnGui();
}

void GameFramework::Update()
{
    if(scene)
    {
        UpdateObject(scene->sceneRoot);
    }

    // if (scene)
    // {
    //     DECREF(scene);
    //     scene = nullptr;
    // }
    // else
    // {
    //     scene = Scene::LoadScene("scenes/test_scene.json");
    //     INCREF(scene);
    // }
    //
    // if (m_renderPipeline)
    // {
    //     delete m_renderPipeline;
    //     m_renderPipeline = nullptr;
    // }
    // else
    // {
    //     m_renderPipeline = new RenderPipeline(m_screenWidth, m_screenHeight, m_window);
    // }

    // Utils::LogInfo("Count %d", SharedObject::m_count);
}

void GameFramework::Render() const
{
    auto mainCamera = CameraComp::GetMainCamera();
    if(mainCamera && scene && m_renderPipeline)
    {
        m_renderPipeline->Render(mainCamera, scene);
    }
    else
    {
        Gui::Render();
        std::cout << "未找到可用摄像机\n";
        Sleep(16);
    }
}

void GameFramework::OnSetFrameBufferSize(GLFWwindow* window, const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    glViewport(0, 0, width, height);
    m_renderPipeline->SetScreenSize(width, height);
}

void GameFramework::InitGame()
{
    m_renderPipeline = std::make_unique<RenderPipeline>(m_screenWidth, m_screenHeight, m_window);
    scene = Scene::LoadScene("scenes/rpgpp_lt_scene_1.0/scene.json");
    // scene = Scene::LoadScene("scenes/test_scene.json");
    // scene = Scene::LoadScene("scenes/ImportTest/scene.json");
    INCREF(scene);

    m_controlPanelUi = std::make_unique<ControlPanelUi>();
}

void GameFramework::ReleaseGame()
{
    if (scene)
    {
        DECREF(scene);
    }
}

