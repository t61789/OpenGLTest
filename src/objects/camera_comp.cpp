#include "camera_comp.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "utils.h"
#include "game_framework.h"
#include "game_resource.h"
#include "gui.h"
#include "render_context.h"
#include "transform_comp.h"

namespace op
{
    using namespace std;

    vec<CameraComp*> CameraComp::m_cameras;

    void CameraComp::Awake()
    {
        m_cameras.push_back(this);
    }

    void CameraComp::OnDestroy()
    {
        remove(m_cameras, this);
    }

    void CameraComp::Start()
    {
        m_targetPosition = GetOwner()->transform->GetWorldPosition();
        m_targetRotation = GetOwner()->transform->GetEulerAngles();
    }

    void CameraComp::Update()
    {
        Matrix4x4 localToWorld = GetOwner()->transform->GetLocalToWorld();
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

        GetOwner()->transform->SetWorldPosition(lerp(GetOwner()->transform->GetWorldPosition(), m_targetPosition, damp));
    
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

        auto r = slerp(GetOwner()->transform->GetRotation(), Quaternion::Euler(m_targetRotation), damp);
        GetOwner()->transform->SetRotation(r);
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
        if(m_cameras.empty())
        {
            return nullptr;
        }

        return m_cameras[0];
    }

    sp<ViewProjInfo> CameraComp::CreateVPMatrix()
    {
        auto cameraLocalToWorld = GetOwner()->transform->GetLocalToWorld();
        cameraLocalToWorld[0][2] = -cameraLocalToWorld[0][2];
        cameraLocalToWorld[1][2] = -cameraLocalToWorld[1][2];
        cameraLocalToWorld[2][2] = -cameraLocalToWorld[2][2];
        auto view = cameraLocalToWorld.Inverse();

        auto aspect = static_cast<float>(GetRC()->screenWidth) / static_cast<float>(GetRC()->screenHeight);
        auto proj = create_projection(fov, aspect, nearClip, farClip);

        auto viewCenter = cameraLocalToWorld.Position();

        return ViewProjInfo::Create(view, proj, viewCenter, true);
    }
    
    sp<ViewProjInfo> CameraComp::CreateShadowVPMatrix(const Vec3 lightDirection)
    {
        ZoneScoped;
        
        float range = 100;
        auto lightForward = -lightDirection.Normalize();
        auto shadowRange = GetRC()->mainLightShadowRange;
        shadowRange = std::min(shadowRange, farClip);
        
        Vec3 forward, right, up;
        forward = lightForward;
        gram_schmidt_ortho(&forward.x, &Vec3::Up().x, &right.x, &up.x);

        auto& cameraL2W = GetOwner()->transform->GetLocalToWorld();
        auto cameraRight = cameraL2W.Right();
        auto cameraUp = cameraL2W.Up();
        auto cameraForward = cameraL2W.Forward();
        auto cameraPos = cameraL2W.Position();
        auto aspect = static_cast<float>(GetRC()->screenWidth) / static_cast<float>(GetRC()->screenHeight);
        
        Vec3 corners[8];
        get_frustum_corners(cameraRight, cameraUp, cameraForward, cameraPos, fov, nearClip, shadowRange, aspect, corners);

        // Gui::Ins()->DrawFrustumPlanes(corners, IM_COL32(0, 255, 0, 255));

        auto shadowWidth = (corners[6] - corners[0]).Magnitude() * 0.5f;
        shadowWidth = std::max(shadowWidth, (corners[4] - corners[5]).Magnitude() * 0.5f);
        shadowWidth = std::max(shadowWidth, (corners[5] - corners[6]).Magnitude() * 0.5f);

        float minU, maxU, minF, maxF;
        float minR = minU = minF = 999999999.0f;
        float maxR = maxU = maxF = -999999999.0f;
        for (auto corner : corners)
        {
            auto r = corner.Dot(right);
            minR = std::min(minR, r);
            maxR = std::max(maxR, r);
            auto u = corner.Dot(up);
            minU = std::min(minU, u);
            maxU = std::max(maxU, u);
            auto f = corner.Dot(forward);
            minF = std::min(minF, f);
            maxF = std::max(maxF, f);
        }

        auto depthRange = maxF - minF + range;
        auto distancePerTexel = shadowWidth * 2 / static_cast<float>(GetRC()->mainLightShadowSize);
        auto rightMove = (minR + maxR) * 0.5f;
        rightMove = std::floor(rightMove / distancePerTexel) * distancePerTexel;
        auto upMove = (minU + maxU) * 0.5f;
        upMove = std::floor(upMove / distancePerTexel) * distancePerTexel;
        auto forwardMove = maxF - depthRange;
        auto shadowCameraPos = right * rightMove + up * upMove + forward * forwardMove;
        
        // 计算阴影矩阵
        auto shadowCameraToWorld = Matrix4x4(
            right.x, up.x, forward.x, shadowCameraPos.x,
            right.y, up.y, forward.y, shadowCameraPos.y,
            right.z, up.z, forward.z, shadowCameraPos.z,
            0, 0, 0, 1);
        auto view = shadowCameraToWorld.Inverse();

        auto proj = create_ortho_projection(shadowWidth, -shadowWidth, shadowWidth, -shadowWidth, depthRange, 0);

        // Gui::Ins()->DrawFrustumPlanes(proj * view);

        return ViewProjInfo::Create(view, proj, shadowCameraPos);
    }
}
