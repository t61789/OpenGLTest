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
    void Render(const Camera* camera) const;
    bool AddEntity(Entity* entity);
    bool RemoveEntity(const Entity* entity);

private:
    int m_ScreenWidth;
    int m_ScreenHeight;
    GLFWwindow* m_Window;
    std::vector<Entity*> m_Entities;
    static void RenderEntity(const ::Entity* entity, const glm::mat4& vpMatrix, const glm::vec3& cameraPositionWS);
};
