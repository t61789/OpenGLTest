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
    delete _commonShader;
    delete _bunnyMesh;
    delete _groundMesh;
    delete _testTexture;
    delete m_renderPipeline;
    delete _bunnyMat;
    delete _groundMat;
    delete _bunny;
    delete _ground;
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
    auto obj = Object::GetObjectPtr(objId);
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
    if(mainCamera != nullptr)
    {
        m_renderPipeline->Render(mainCamera);
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
    
    // Shader
    _commonShader = new Shader("../assets/TestVertShader.vert", "../assets/TestFragShader.frag");
    _commonShader->SetInt("tex", 0);

    // Texture
    _testTexture = new Texture("../assets/o2.png");

    // Material
    _bunnyMat = new Material();
    _bunnyMat->SetFloatValue("_ShowTex", 0);

    _groundMat = new Material();
    _groundMat->SetTextureValue("_MainTex", _testTexture);
    _groundMat->SetFloatValue("_ShowTex", 1);

    // Mesh
    // _bunnyMesh = Mesh::LoadFromFile("../assets/stanford-bunny.obj");
    _bunnyMesh = Mesh::LoadFromFile("../assets/beetle.obj");
    // _bunnyMesh = Mesh::LoadFromFile("../assets/beast.obj");
    
    float vertices[] = {
        0.5f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
       -0.5f,  0.5f, 0.0f,
    };
    float texcoords[] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    _groundMesh = new Mesh(vertices, nullptr, texcoords, nullptr, indices, sizeof(indices) / sizeof(unsigned int));

    // Entity
    _bunny = new Entity(_commonShader, _bunnyMesh, _bunnyMat);
    _bunny->m_scale = glm::vec3(10, 10, 10);
    _bunny->m_rotation = glm::vec3(0, 0, 0);
    m_renderPipeline->AddEntity(_bunny);

    _ground = new Entity(_commonShader, _groundMesh, _groundMat);
    _ground->m_scale = glm::vec3(10, 10, 1);
    _ground->m_rotation = glm::vec3(-90, 0, 0);
    m_renderPipeline->AddEntity(_ground);

    m_scene = new Scene("F://Shit.json");
}

