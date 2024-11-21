#pragma once

#include <glfw3.h>
#include "Material.h"
#include "Scene.h"

class RenderContext
{
public:
    glm::mat4 m_vpMatrix;
    glm::vec3 m_cameraPositionWS;
    glm::vec3 m_lightDirection;
};

class RenderPipeline
{
public:
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void SetScreenSize(int width, int height);
    void Render(RESOURCE_ID cameraId, const Scene* scene) const;

private:
    int m_screenWidth;
    int m_screenHeight;
    GLFWwindow* m_window;
};
