#pragma once
#include "Object.h"

class Camera : public Object
{
public:
    Camera();
    ~Camera() override;
    void Update() override;
    void LoadFromJson(const nlohmann::json& objJson) override;
    float fov = 45.0f;
    float nearClip = 0.1f;
    float farClip = 30.0f;
    static Camera* GetMainCamera();
    
private:
    glm::vec3 m_targetPosition = {};
    glm::vec3 m_targetRotation = {};
    static std::vector<Camera*> s_cameras;
};
