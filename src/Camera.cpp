#include "Camera.h"

#include "GameFramework.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

std::vector<Camera*> Camera::s_cameras;

Camera::Camera()
{
    s_cameras.push_back(this);
}

Camera::~Camera()
{
    s_cameras.erase(std::remove(s_cameras.begin(), s_cameras.end(), this), s_cameras.end());
}

void Camera::Update()
{
    auto localToWorld = GetLocalToWorld();
    glm::vec3 forward = localToWorld * glm::vec4(0, 0, -1.0f, 0.0f);
    glm::vec3 right = localToWorld * glm::vec4(1.0f, 0, 0, 0.0f);

    float moveSpeed = 3;
    float rotateSpeed = 80;
    float damp = 0.07f;

    GameFramework* gameFramework = GameFramework::GetInstance();

    if(gameFramework->KeyPressed(GLFW_KEY_W))
    {
        m_targetPosition += forward * gameFramework->GetDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_S))
    {
        m_targetPosition += -forward * gameFramework->GetDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_A))
    {
        m_targetPosition += -right * gameFramework->GetDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_D))
    {
        m_targetPosition += right * gameFramework->GetDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_E))
    {
        m_targetPosition += glm::vec3(0, 1.0f, 0) * gameFramework->GetDeltaTime() * moveSpeed;
    }

    if(gameFramework->KeyPressed(GLFW_KEY_Q))
    {
        m_targetPosition += glm::vec3(0, -1.0f, 0) * gameFramework->GetDeltaTime() * moveSpeed;
    }

    position = lerp(position, m_targetPosition, damp);
    
    if(gameFramework->KeyPressed(GLFW_KEY_UP))
    {
        m_targetRotation.x += gameFramework->GetDeltaTime() * rotateSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_DOWN))
    {
        m_targetRotation.x += -gameFramework->GetDeltaTime() * rotateSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_LEFT))
    {
        m_targetRotation.y += gameFramework->GetDeltaTime() * rotateSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_RIGHT))
    {
        m_targetRotation.y += -gameFramework->GetDeltaTime() * rotateSpeed;
    }

    rotation = lerp(rotation, m_targetRotation, damp);
}

void Camera::LoadFromJson(const nlohmann::json& objJson)
{
    Object::LoadFromJson(objJson);

    if(objJson.contains("fov"))
    {
        fov = objJson["fov"].get<float>();
    }

    if(objJson.contains("nearClip"))
    {
        nearClip = objJson["nearClip"].get<float>();
    }

    if(objJson.contains("farClip"))
    {
        farClip = objJson["farClip"].get<float>();
    }

    m_targetPosition = this->position;
    m_targetRotation = this->rotation;
}

Camera* Camera::GetMainCamera()
{
    if(s_cameras.empty())
    {
        return nullptr;
    }

    return s_cameras[0];
}


