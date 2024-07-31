#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include "Shader.h"
#include "glm.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "Mesh.h"
#include "Object.h"
#include "../lib/stb_image.h"

GLFWwindow* g_Window;

Object g_MainCamera;
Object g_Cube;

Shader* g_TestShader;
Mesh* g_TestMesh;
GLuint g_TestTexture;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << width << " " << height << "\n";
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

GLuint loadTexture(char const* path)
{
    int width, height, nChannels;
    stbi_uc* data;
    try
    {
        data = stbi_load(path, &width, &height, &nChannels, 0);
        if(!data)
        {
            throw std::runtime_error("ERROR>> Failed to load texture: " + std::string(path));
        }
    }
    catch(std::exception)
    {
        throw;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO may cause performance problem
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); // TODO errors here
    
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
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

    g_Window = glfwCreateWindow(800, 600, "YeahTitle", nullptr, nullptr);
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
        std::cout << "Failed to initialize GLAD\n";
        return false;
    }

    stbi_set_flip_vertically_on_load(true);
    
    int maxVertexAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttributes);
    std::cout << "Max vertex attributes: " << maxVertexAttributes << "\n";

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
    g_TestShader->setInt("tex", 0);

    // Texture
    g_TestTexture = loadTexture("../assets/o2.png");

    // Mesh
    float position[] = {
        0.5f,  0.5f, 0.0f, // top right
        0.5f, -0.5f, 0.0f, // bottom right
       -0.5f, -0.5f, 0.0f, // bottom left
       -0.5f,  0.5f, 0.0f, // top left 
    };
    float texcoord[] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };
    float color[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    g_TestMesh = new Mesh(position, texcoord, color, indices, 6);
}

void releaseAll()
{
    delete g_TestShader;
    delete g_TestMesh;
}

void update()
{
    
}

int process()
{
    if(!initFrame())
    {
        return -1;
    }

    initGame();

    // Render loop
    double timeCount = 0;
    int frameCount = 0;
    while(!glfwWindowShouldClose(g_Window))
    {
        frameCount ++;
        double curTime = glfwGetTime();
        if(curTime - timeCount > 1)
        {
            std::cout << "Frame Rate: " << frameCount / (curTime - timeCount) << "\n";
            frameCount = 0;
            timeCount = curTime;
        }
        
        processInput(g_Window);

        // Render
        glClearColor(0.2f, 0.2f, 0.2f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, g_TestTexture);

        g_TestShader->use();

        g_TestMesh->use();
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(g_Window);
        glfwPollEvents();
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
    catch(std::exception &e)
    {
        std::cout << e.what() << "\n";
    }
}
