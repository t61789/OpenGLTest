#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/euler_angles.hpp>

#include "GameFramework.h"

std::vector<OBJECT_ID> Camera::s_cameras;

Camera::Camera()
{
    s_cameras.push_back(m_id);
}

Camera::~Camera()
{
    for(size_t i = 0; i < s_cameras.size(); i++)
    {
        if(s_cameras[i] != m_id)
        {
            continue;
        }
        
        s_cameras[i] = s_cameras[s_cameras.size() - 1];
        s_cameras.pop_back();
        return;
    }
}

void Camera::Update()
{
    auto localToWorld = GetLocalToWorld();
    glm::vec3 forward = localToWorld * glm::vec4(0, 0, -1.0f, 0.0f);
    glm::vec3 right = localToWorld * glm::vec4(1.0f, 0, 0, 0.0f);

    float moveSpeed = 3;
    float rotateSpeed = 80;
    float damp = 0.07f;

    if(GameFramework::instance->KeyPressed(GLFW_KEY_W))
    {
        m_targetPosition += forward * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_S))
    {
        m_targetPosition += -forward * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_A))
    {
        m_targetPosition += -right * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_D))
    {
        m_targetPosition += right * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_E))
    {
        m_targetPosition += glm::vec3(0, 1.0f, 0) * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }

    if(GameFramework::instance->KeyPressed(GLFW_KEY_Q))
    {
        m_targetPosition += glm::vec3(0, -1.0f, 0) * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }

    m_position = lerp(m_position, m_targetPosition, damp);
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_UP))
    {
        m_targetRotation.x += GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_DOWN))
    {
        m_targetRotation.x += -GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_LEFT))
    {
        m_targetRotation.y += GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_RIGHT))
    {
        m_targetRotation.y += -GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }

    m_rotation = lerp(m_rotation, m_targetRotation, damp);
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

    m_targetPosition = this->m_position;
    m_targetRotation = this->m_rotation;
}

Camera* Camera::GetMainCamera()
{
    if(s_cameras.empty())
    {
        return nullptr;
    }

    return (Camera*)GetObjectPtr(s_cameras[0]);
}


