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
#include <tracy/Tracy.hpp>
#include "GLFW/glfw3.h"

#include "game_resource.h"
#include "render_pipeline.h"
#include "render/render_target_pool.h"
#include "render/gl/gl_state.h"

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

    GameFramework::GameFramework() = default;

    GameFramework::~GameFramework()
    {
        ReleaseGame();
        
        glfwDestroyWindow(m_window);
        glfwTerminate();
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
    }

    bool GameFramework::KeyPressed(const int glfwKey) const
    {
        return glfwGetKey(m_window, glfwKey) == GLFW_PRESS;
    }

    Scene* GameFramework::GetMainScene() const
    {
        return m_scene.get();
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
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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
                GetGR()->onFrameBufferResize.Invoke(window, width, height);
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
        Gui::Ins()->BeginFrame();

        auto& time = GetGR()->time;
        auto curFrameTime = static_cast<float>(glfwGetTime());
        time.frame += 1;
        time.deltaTime = max(curFrameTime - time.time, 0.000001f);
        time.time = curFrameTime;
    }

    void GameFramework::FrameEnd()
    {
        GetGR()->onFrameEnd.Invoke();
        GetRC()->renderTargetPool->TryRecycle();
        
        FrameMark;
    }

    void GameFramework::BeforeUpdate()
    {
        Gui::Ins()->BeforeUpdate();
    }

    void GameFramework::Update()
    {
        ZoneScoped;

        if(GetMainScene())
        {
            auto indices = GetMainScene()->GetIndices();
            auto comps = indices->GetAllComps();
            for (const auto& compPtr : comps)
            {
                auto comp = compPtr.lock();
                assert(comp);
                
                if (!comp->IsStarted())
                {
                    comp->Start();
                    comp->SetIsStarted(true);
                    comp->SetEnable(true);
                }
                
                comp->Update();
            }
        }

        // if (scene)
        // {
        //     DECREF(scene)
        //     scene = nullptr;
        // }
        // else
        // {
        //     scene = Scene::LoadScene("scenes/test_scene.json");
        //     INCREF(scene)
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
            Gui::Ins()->Render(nullptr);
            std::cout << "未找到可用摄像机\n";
            Sleep(16);
        }
    }

    void GameFramework::OnSetFrameBufferResize(GLFWwindow* window, const int width, const int height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
        glViewport(0, 0, width, height);
        m_renderPipeline->SetScreenSize(width, height);
    }

    void GameFramework::InitGame()
    {
        m_glState = new GlState();
        m_gameResource = new GameResource();
        m_onFrameBufferResizeHandler = GetGR()->onFrameBufferResize.Add(this, &GameFramework::OnSetFrameBufferResize); // TODO resize
        
        m_gui = new Gui();
        m_builtInRes = new BuiltInRes();
        m_renderPipeline = new RenderPipeline(m_screenWidth, m_screenHeight, m_window);
        // m_scene = Scene::LoadScene("scenes/rpgpp_lt_scene_1.0/scene.json");
        m_scene = Scene::LoadScene("scenes/test_scene/test_scene.json");
        // m_scene = Scene::LoadScene("scenes/ImportTest/scene.json");
        // m_scene = Scene::LoadScene("scenes/Scene_A/scene.json");
        // m_scene = Scene::LoadScene("scenes/HDRP_template/scene.json");
    }

    void GameFramework::ReleaseGame()
    {
        m_scene.reset();
        delete m_renderPipeline;
        delete m_builtInRes;
        delete m_gui;
        GetGR()->onFrameBufferResize.Remove(m_onFrameBufferResizeHandler);
        delete m_gameResource;
        delete m_glState;
    }
}
