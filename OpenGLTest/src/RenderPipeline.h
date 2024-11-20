#pragma once
#include <vector>

#include "Entity.h"
#include <glfw3.h>
#include "Material.h"
#include "Camera.h"

class RenderPipeline
{
public:
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void SetScreenSize(int width, int height);
    void Render(RESOURCE_ID cameraId, RESOURCE_ID sceneRootId) const;

private:
    int m_ScreenWidth;
    int m_ScreenHeight;
    GLFWwindow* m_Window;
};
