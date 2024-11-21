#include "GameFramework.h"

#include <iostream>

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
    delete _renderPipeline;
    delete _bunnyMat;
    delete _groundMat;
    delete _bunny;
    delete _ground;
    delete _camera;

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

void GameFramework::Update()
{
    _camera->Update();
    
    // _entity->rotation.y += GetDeltaTime() * 60.0f;
}

void GameFramework::Render()
{
    _renderPipeline->Render(_camera);
}

void GameFramework::FRAME_BUFFER_SIZE_CALL_BACK(GLFWwindow* window, int width, int height)
{
    std::cout << width << " " << height << "\n";
    instance->_screenWidth = width;
    instance->_screenHeight = height;
    glViewport(0, 0, width, height);
    instance->_renderPipeline->SetScreenSize(width, height);
}

void GameFramework::InitGame()
{
    _camera = new Camera(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0));
    _renderPipeline = new RenderPipeline(_screenWidth, _screenHeight, _window);
    
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
    _bunnyMesh = Mesh::LoadFromFile("../assets/stanford-bunny.obj");
    
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
    _renderPipeline->AddEntity(_bunny);

    _ground = new Entity(_commonShader, _groundMesh, _groundMat);
    _ground->m_scale = glm::vec3(10, 10, 1);
    _ground->m_rotation = glm::vec3(-90, 0, 0);
    _renderPipeline->AddEntity(_ground);

    m_scene = new Scene("F://Shit.json");

    auto sceneRoot = Object::GetObjectPtr(m_scene->m_sceneRoot);
    for (auto& child : sceneRoot->m_children)
    {
        std::cout << Object::GetObjectPtr(child)->m_name << std::endl;
    }
}

