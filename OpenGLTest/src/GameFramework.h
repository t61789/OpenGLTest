#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <glfw3.h>
#include "gtc/type_ptr.hpp"

#include <iostream>

#include "Camera.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Object.h"
#include "Material.h"
#include "RenderPipeline.h"
#include "Entity.h"
#include "Event.h"

class GameFramework
{
public:
    static GameFramework* instance;

    GameFramework();
    ~GameFramework();

    bool Init();
    void StartGameLoop();

    float GetDeltaTime();
    float GetCurFrameTime();
    bool KeyPressed(int glfwKey) const;
    
private:
    GLFWwindow* _window;

    Shader* _testShader;
    Mesh* _testMesh;
    Texture* _testTexture;
    Material* _testMat;
    Entity* _entity;
    Camera* _camera;

    RenderPipeline* _renderPipeline;

    int _screenWidth = 800;
    int _screenHeight = 600;

    float _deltaTime;
    float _curFrameTime;

    void InitGame();
    bool InitFrame();
    bool InitGlfw();
    void ProcessInput();
    void Update();
    void Render();
    static void FRAME_BUFFER_SIZE_CALL_BACK(GLFWwindow* window, int width, int height);
};
