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
    delete _testShader;
    delete _testMesh;
    delete _testTexture;
    delete _renderPipeline;
    delete _testMat;
    delete _entity;
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
    while (!glfwWindowShouldClose(_window))
    {
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
    
    // Shader
    _testShader = new Shader("../assets/TestVertShader.vert", "../assets/TestFragShader.frag");
    _testShader->SetInt("tex", 0);

    // Texture
    _testTexture = new Texture("../assets/o2.png");

    // Material
    _testMat = new Material();
    _testMat->SetTextureValue("tex", _testTexture);

    // Mesh
    float position[] = {
        // 下
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, -0.5f,
    
        // 后
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
    
        // 左
        -0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
    
        // 右
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
    
        // 前
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
    
        // 上
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
    };
    float texcoord[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3,
    
        4, 5, 6,
        4, 6, 7,
    
        8, 9, 10,
        8, 10, 11,
    
        12, 13, 14,
        12, 14, 15,
    
        16, 17, 18,
        16, 18, 19,
    
        20, 21, 22,
        20, 22, 23
    };

    _testMesh = new Mesh(position, texcoord, nullptr, indices, sizeof(indices) / sizeof(int));

    _renderPipeline = new RenderPipeline(_screenWidth, _screenHeight, _window);

    _entity = new Entity(_testShader, _testMesh, _testMat);
    _entity->rotation = glm::vec3(0, 0, 0);
    _renderPipeline->AddEntity(_entity);
}

