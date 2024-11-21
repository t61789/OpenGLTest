#pragma once

#include "Entity.h"
#include "Material.h"
#include "Scene.h"

class RenderContext
{
public:
    glm::mat4 vpMatrix;
    glm::vec3 cameraPositionWS;
    glm::vec3 mainLightDirection;
    glm::vec3 mainLightColor;
    glm::vec3 ambientLightColor;
};

class RenderPipeline
{
public:
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void setScreenSize(int width, int height);
    void render(RESOURCE_ID cameraId, const Scene* scene);

private:
    int m_screenWidth;
    int m_screenHeight;
    GLFWwindow* m_window;
    GLuint m_frameBuffer = -1;
    GLuint m_cameraColorAttachment = -1;
    GLuint m_cameraDepthAttachment = -1;

    RESOURCE_ID m_fullScreenQuad;
    RESOURCE_ID m_blitShader;

    void _renderEntity(const Entity* entity, const RenderContext& renderContext);
    void _updateCameraAttachments();
};
