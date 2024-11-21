#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/euler_angles.hpp>

#include "GameFramework.h"

std::vector<OBJECT_ID> Camera::s_cameras;

Camera::Camera()
{
    s_cameras.push_back(id);
}

Camera::~Camera()
{
    for(size_t i = 0; i < s_cameras.size(); i++)
    {
        if(s_cameras[i] != id)
        {
            continue;
        }
        
        s_cameras[i] = s_cameras[s_cameras.size() - 1];
        s_cameras.pop_back();
        return;
    }
}

void Camera::update()
{
    auto localToWorld = getLocalToWorld();
    glm::vec3 forward = localToWorld * glm::vec4(0, 0, -1.0f, 0.0f);
    glm::vec3 right = localToWorld * glm::vec4(1.0f, 0, 0, 0.0f);

    float moveSpeed = 3;
    float rotateSpeed = 80;
    float damp = 0.07f;

    GameFramework* gameFramework = GameFramework::getInstance();

    if(gameFramework->keyPressed(GLFW_KEY_W))
    {
        m_targetPosition += forward * gameFramework->getDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->keyPressed(GLFW_KEY_S))
    {
        m_targetPosition += -forward * gameFramework->getDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->keyPressed(GLFW_KEY_A))
    {
        m_targetPosition += -right * gameFramework->getDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->keyPressed(GLFW_KEY_D))
    {
        m_targetPosition += right * gameFramework->getDeltaTime() * moveSpeed;
    }
    
    if(gameFramework->keyPressed(GLFW_KEY_E))
    {
        m_targetPosition += glm::vec3(0, 1.0f, 0) * gameFramework->getDeltaTime() * moveSpeed;
    }

    if(gameFramework->keyPressed(GLFW_KEY_Q))
    {
        m_targetPosition += glm::vec3(0, -1.0f, 0) * gameFramework->getDeltaTime() * moveSpeed;
    }

    position = lerp(position, m_targetPosition, damp);
    
    if(gameFramework->keyPressed(GLFW_KEY_UP))
    {
        m_targetRotation.x += gameFramework->getDeltaTime() * rotateSpeed;
    }
    
    if(gameFramework->keyPressed(GLFW_KEY_DOWN))
    {
        m_targetRotation.x += -gameFramework->getDeltaTime() * rotateSpeed;
    }
    
    if(gameFramework->keyPressed(GLFW_KEY_LEFT))
    {
        m_targetRotation.y += gameFramework->getDeltaTime() * rotateSpeed;
    }
    
    if(gameFramework->keyPressed(GLFW_KEY_RIGHT))
    {
        m_targetRotation.y += -gameFramework->getDeltaTime() * rotateSpeed;
    }

    rotation = lerp(rotation, m_targetRotation, damp);
}

void Camera::loadFromJson(const nlohmann::json& objJson)
{
    Object::loadFromJson(objJson);

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

RESOURCE_ID Camera::GetMainCamera()
{
    if(s_cameras.empty())
    {
        return UNDEFINED_RESOURCE;
    }

    for (size_t i = 0; i < s_cameras.size(); ++i)
    {
        auto result = ResourceMgr::GetPtr<Camera>(s_cameras[i]);
        if(result != nullptr)
        {
            return s_cameras[i];
        }
    }

    return UNDEFINED_RESOURCE;
}


