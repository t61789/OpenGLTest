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
    float tonemappingExposureMultiplier = 1;
};

class RenderPipeline
{
public:
    RenderPipeline(int width, int height, GLFWwindow* window);
    ~RenderPipeline();
    void setScreenSize(int width, int height);
    void render(RESOURCE_ID cameraId, const Scene* scene);

private:
    size_t m_screenWidth;
    size_t m_screenHeight;
    GLFWwindow* m_window;
    GLuint m_backBuffer = -1;
    RESOURCE_ID m_cameraColorAttachment0 = UNDEFINED_RESOURCE;
    RESOURCE_ID m_cameraColorAttachment1 = UNDEFINED_RESOURCE;
    RESOURCE_ID m_cameraDepthAttachment = UNDEFINED_RESOURCE;

    RESOURCE_ID m_fullScreenQuad = UNDEFINED_RESOURCE;
    RESOURCE_ID m_blitShader = UNDEFINED_RESOURCE;
    RESOURCE_ID m_deferredShadingShader = UNDEFINED_RESOURCE;

    void _clearAttachmentsPass();
    void _renderScenePass(RESOURCE_ID cameraId, const Scene* scene, RenderContext& renderContext);
    void _renderEntity(const Entity* entity, const RenderContext& renderContext);
    void _deferredShadingPass();
    void _finalBlitPass(const RenderContext& renderContext);
    bool _updateCameraAttachments();
};
