#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <iostream>

#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Object.h"
#include "Material.h"
#include "RenderPipeline.h"
#include "Entity.h"

class GameFramework
{
public:
    static GameFramework* instance;
    
    GameFramework();
    ~GameFramework();

    bool Init();
    void StartGameLoop();
    
private:
    GLFWwindow* _window;

    Object _mainCamera;
    Object _cube;

    Shader* _testShader;
    Mesh* _testMesh;
    Texture* _testTexture;
    Material* _testMat;
    Entity* _entity;

    RenderPipeline* _renderPipeline;

    int _screenWidth = 800;
    int _screenHeight = 600;

    void InitGame();
    bool InitFrame();
    bool InitGlfw();
    void ProcessInput();
    void Update(const double& time, const double& deltaTime);
    void Render();
    static void FRAME_BUFFER_SIZE_CALL_BACK(GLFWwindow* window, int width, int height);
};
