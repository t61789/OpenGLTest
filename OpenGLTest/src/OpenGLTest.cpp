#include "OpenGLTest.h"

GLFWwindow* g_Window;

Object g_MainCamera;
Object g_Cube;

Shader* g_TestShader;
Mesh* g_TestMesh;
Texture* g_TestTexture;
Material* g_TestMat;
Entity* g_Entity;

RenderPipeline* g_RenderPipeline;

int g_ScreenWidth = 800;
int g_ScreenHeight = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << width << " " << height << "\n";
    g_ScreenWidth = width;
    g_ScreenHeight = height;
    glViewport(0, 0, width, height);
    g_RenderPipeline->SetScreenSize(width, height);
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void genMesh(GLuint* VAO, GLuint* VBO, GLuint* EBO)
{
    // vertices
    float vertices[] = {
        // positions          // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
       -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);

    glBindVertexArray(*VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool initGlfw()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_Window = glfwCreateWindow(g_ScreenWidth, g_ScreenHeight, "YeahTitle", nullptr, nullptr);
    if(!g_Window)
    {
        std::cout << "Failed to create a window\n";
        glfwTerminate();

        return false;
    }
    glfwMakeContextCurrent(g_Window);
    glfwSetFramebufferSizeCallback(g_Window, framebuffer_size_callback);

    return true;
}

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

bool initFrame()
{
    if(!initGlfw())
    {
        return false;
    }

    if(!initGl())
    {
        return false;
    }

    return true;
}

void initGame()
{
    // Shader
    g_TestShader = new Shader("../assets/TestVertShader.vert", "../assets/TestFragShader.frag");
    g_TestShader->SetInt("tex", 0);

    // Texture
    g_TestTexture = new Texture("../assets/o2.png");

    // Material
    g_TestMat = new Material();
    g_TestMat->SetTextureValue("tex", g_TestTexture);

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

    g_TestMesh = new Mesh(position, texcoord, nullptr, indices, sizeof(indices) / sizeof(int));

    g_RenderPipeline = new RenderPipeline(g_ScreenWidth, g_ScreenHeight, g_Window);

    g_Entity = new Entity(g_TestShader, g_TestMesh, g_TestMat);
    g_Entity->rotation = glm::vec3(30, 0, 0);
    g_RenderPipeline->AddEntity(g_Entity);
}

void releaseAll()
{
    delete g_TestShader;
    delete g_TestMesh;
    delete g_TestTexture;
    delete g_RenderPipeline;
    delete g_TestMat;
    delete g_Entity;
}

void update(const double& time, const double& deltaTime)
{
    g_Entity->rotation.y += deltaTime * 60.0f;
}

void render()
{
    g_RenderPipeline->Render();
}

int process()
{
    if (!initFrame())
    {
        return -1;
    }

    initGame();

    // Render loop
    double timeCount = 0;
    double preFrameTime = 0;
    int frameCount = 0;
    while (!glfwWindowShouldClose(g_Window))
    {
        frameCount++;
        double curFrameTime = glfwGetTime();
        if (curFrameTime - timeCount > 1)
        {
            std::cout << "Frame Rate: " << frameCount / (curFrameTime - timeCount) << "\n";
            frameCount = 0;
            timeCount = curFrameTime;
        }

        processInput(g_Window);

        update(curFrameTime, curFrameTime - preFrameTime);

        // Render
        render();
        
        preFrameTime = curFrameTime;
    }

    releaseAll();
    glfwTerminate();
    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        process();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
}
