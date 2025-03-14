#include "CameraComp.h"

#include "glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtc/quaternion.hpp"

#include "Utils.h"
#include "GameFramework.h"
#include "TransformComp.h"

std::vector<CameraComp*> CameraComp::s_cameras;

CameraComp::CameraComp()
{
    s_cameras.push_back(this);
}

CameraComp::~CameraComp()
{
    s_cameras.erase(std::remove(s_cameras.begin(), s_cameras.end(), this), s_cameras.end());
}

void CameraComp::Update()
{
    auto localToWorld = owner->transform->GetLocalToWorld();
    glm::vec3 forward = localToWorld * glm::vec4(0, 0, -1.0f, 0.0f);
    glm::vec3 right = localToWorld * glm::vec4(1.0f, 0, 0, 0.0f);

    float moveSpeed = 3;
    float rotateSpeed = 80;
    float damp = 0.07f;

    GameFramework* gameFramework = GameFramework::GetInstance();
    auto deltaTime = Time::GetInstance()->deltaTime;

    if(gameFramework->KeyPressed(GLFW_KEY_W))
    {
        m_targetPosition += forward * deltaTime * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_S))
    {
        m_targetPosition += -forward * deltaTime * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_A))
    {
        m_targetPosition += -right * deltaTime * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_D))
    {
        m_targetPosition += right * deltaTime * moveSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_E))
    {
        m_targetPosition += glm::vec3(0, 1.0f, 0) * deltaTime * moveSpeed;
    }

    if(gameFramework->KeyPressed(GLFW_KEY_Q))
    {
        m_targetPosition += glm::vec3(0, -1.0f, 0) * deltaTime * moveSpeed;
    }

    owner->transform->SetPosition(lerp(owner->transform->GetPosition(), m_targetPosition, damp));
    
    if(gameFramework->KeyPressed(GLFW_KEY_UP))
    {
        m_targetRotation.x += deltaTime * rotateSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_DOWN))
    {
        m_targetRotation.x += -deltaTime * rotateSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_LEFT))
    {
        m_targetRotation.y += deltaTime * rotateSpeed;
    }
    
    if(gameFramework->KeyPressed(GLFW_KEY_RIGHT))
    {
        m_targetRotation.y += -deltaTime * rotateSpeed;
    }

    auto r = normalize(lerp(owner->transform->GetRotation(), glm::quat(glm::radians(m_targetRotation)), damp));
    owner->transform->SetRotation(r);
}

void CameraComp::LoadFromJson(const nlohmann::json& objJson)
{
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

    m_targetPosition = owner->transform->GetPosition();
    m_targetRotation = degrees(eulerAngles(owner->transform->GetRotation()));
}

CameraComp* CameraComp::GetMainCamera()
{
    if(s_cameras.empty())
    {
        return nullptr;
    }

    return s_cameras[0];
}


