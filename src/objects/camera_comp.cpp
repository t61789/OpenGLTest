#include "camera_comp.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "utils.h"
#include "game_framework.h"
#include "game_resource.h"
#include "transform_comp.h"

namespace op
{
    using namespace std;

    vector<CameraComp*> CameraComp::s_cameras;

    CameraComp::CameraComp()
    {
        s_cameras.push_back(this);
    }

    CameraComp::~CameraComp()
    {
        s_cameras.erase(remove(s_cameras.begin(), s_cameras.end(), this), s_cameras.end());
    }

    void CameraComp::Start()
    {
        m_targetPosition = owner->transform->GetWorldPosition();
        m_targetRotation = owner->transform->GetEulerAngles();
    }

    void CameraComp::Update()
    {
        Matrix4x4 localToWorld = owner->transform->GetLocalToWorld();
        Vec3 forward = localToWorld.Forward();
        Vec3 right = localToWorld.Right();
        Vec3 up = localToWorld.Up();

        float moveSpeed = 6;
        float accleration = 8;
        float rotateSpeed = 125;
        float damp = 0.07f;

        GameFramework* gameFramework = GameFramework::Ins();
        auto deltaTime = GetGR()->time.deltaTime;

        if(gameFramework->KeyPressed(GLFW_KEY_LEFT_SHIFT))
        {
            m_curSpeedAdd += deltaTime * accleration;
            moveSpeed += m_curSpeedAdd;
        }
        else
        {
            m_curSpeedAdd = 0;
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
            m_targetPosition += up * deltaTime * moveSpeed;
        }

        if(gameFramework->KeyPressed(GLFW_KEY_Q))
        {
            m_targetPosition -= up * deltaTime * moveSpeed;
        }

        owner->transform->SetWorldPosition(lerp(owner->transform->GetWorldPosition(), m_targetPosition, damp));
    
        if(gameFramework->KeyPressed(GLFW_KEY_UP))
        {
            m_targetRotation.x -= deltaTime * rotateSpeed;
        }
    
        if(gameFramework->KeyPressed(GLFW_KEY_DOWN))
        {
            m_targetRotation.x += deltaTime * rotateSpeed;
        }
    
        if(gameFramework->KeyPressed(GLFW_KEY_LEFT))
        {
            m_targetRotation.y -= deltaTime * rotateSpeed;
        }
    
        if(gameFramework->KeyPressed(GLFW_KEY_RIGHT))
        {
            m_targetRotation.y += deltaTime * rotateSpeed;
        }

        auto r = slerp(owner->transform->GetRotation(), Quaternion::Euler(m_targetRotation), damp);
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
