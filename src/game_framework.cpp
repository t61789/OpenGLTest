#include "game_framework.h"

#include <iostream>

#include "imgui.h"

#include "scene.h"
#include "gui.h"
#include "windows.h"
#include "built_in_res.h"
#include "objects/camera_comp.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace op
{
    static bool InitGl()
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

    GameFramework::GameFramework()
    {
        m_setFrameBufferSizeHandler = Utils::s_setFrameBufferSizeEvent.Add(this, &GameFramework::OnSetFrameBufferSize);
    }

    GameFramework::~GameFramework()
    {
        Utils::s_setFrameBufferSizeEvent.Remove(m_setFrameBufferSizeHandler);

        ReleaseGame();
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

    Scene* GameFramework::GetMainScene() const
    {
        return m_scene;
    }

    bool GameFramework::InitFrame()
    {
        if(!InitGlfw())
        {
            return false;
        }

        if(!InitGl())
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
        Gui::GetInstance()->BeginFrame();

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
                if (!comp->IsStarted())
                {
                    comp->Start();
                }
                
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
        Gui::GetInstance()->BeforeUpdate();
    }

    void GameFramework::Update()
    {
        if(GetMainScene())
        {
            UpdateObject(GetMainScene()->sceneRoot);
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
        if(mainCamera && GetMainScene() && m_renderPipeline)
        {
            m_renderPipeline->Render(mainCamera, GetMainScene());
        }
        else
        {
            Gui::GetInstance()->Render(nullptr);
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
        m_time = std::make_unique<Time>();
        m_gui = std::make_unique<Gui>();
        m_builtInRes = std::make_unique<BuiltInRes>();
        m_renderPipeline = std::make_unique<RenderPipeline>(m_screenWidth, m_screenHeight, m_window);
        // scene = Scene::LoadScene("scenes/rpgpp_lt_scene_1.0/scene.json");
        m_scene = Scene::LoadScene("scenes/test_scene/test_scene.json");
        // scene = Scene::LoadScene("scenes/ImportTest/scene.json");
        INCREF(m_scene);
    }

    void GameFramework::ReleaseGame()
    {
        if (m_scene)
        {
            DECREF(m_scene);
        }
    }
}
