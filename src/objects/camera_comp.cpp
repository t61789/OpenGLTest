#include "camera_comp.h"

#include "glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtc/quaternion.hpp"

#include "utils.h"
#include "game_framework.h"
#include "transform_comp.h"

namespace op
{
    using namespace std;
    using namespace glm;

    vector<CameraComp*> CameraComp::s_cameras;

    CameraComp::CameraComp()
    {
        s_cameras.push_back(this);
    }

    CameraComp::~CameraComp()
    {
        s_cameras.erase(remove(s_cameras.begin(), s_cameras.end(), this), s_cameras.end());
    }

    void CameraComp::Awake()
    {
        m_targetPosition = owner->transform->GetPosition();
        m_targetRotation = owner->transform->GetEulerAngles();
    }

    void CameraComp::Update()
    {
        auto localToWorld = owner->transform->GetLocalToWorld();
        vec3 forward = localToWorld * vec4(0, 0, -1.0f, 0.0f);
        vec3 right = localToWorld * vec4(1.0f, 0, 0, 0.0f);

        float moveSpeed = 6;
        float rotateSpeed = 155;
        float damp = 0.07f;

        GameFramework* gameFramework = GameFramework::GetInstance();
        auto deltaTime = Time::GetInstance()->deltaTime;

        if(gameFramework->KeyPressed(GLFW_KEY_LEFT_SHIFT))
        {
            moveSpeed *= 3;
        }

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
            m_targetPosition += vec3(0, 1.0f, 0) * deltaTime * moveSpeed;
        }

        if(gameFramework->KeyPressed(GLFW_KEY_Q))
        {
            m_targetPosition += vec3(0, -1.0f, 0) * deltaTime * moveSpeed;
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

        auto r = slerp(owner->transform->GetRotation(), quat(radians(m_targetRotation)), damp);
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
    }

    CameraComp* CameraComp::GetMainCamera()
    {
        if(s_cameras.empty())
        {
            return nullptr;
        }

        return s_cameras[0];
    }
}
