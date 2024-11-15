﻿#pragma once

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
#include "Scene.h"

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
    float GetFrameCount();
    bool KeyPressed(int glfwKey) const;
    
private:
    GLFWwindow* _window;

    Shader* _commonShader;
    Mesh* _bunnyMesh;
    Mesh* _groundMesh;
    Texture* _testTexture;
    Material* _bunnyMat;
    Material* _groundMat;
    Entity* _bunny;
    Entity* _ground;
    Camera* _camera;

    Scene* m_scene;

    RenderPipeline* _renderPipeline;

    int _screenWidth = 800;
    int _screenHeight = 600;

    float _deltaTime;
    float _curFrameTime;
    int _frameCount;

    void InitGame();
    bool InitFrame();
    bool InitGlfw();
    void ProcessInput();
    void Update();
    void Render();
    static void FRAME_BUFFER_SIZE_CALL_BACK(GLFWwindow* window, int width, int height);
};
