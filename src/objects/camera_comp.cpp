#include "camera_comp.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "utils.h"
#include "game_framework.h"
#include "game_resource.h"
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

        return ViewProjInfo::Create(view, proj, viewCenter);
    }
    
    sp<ViewProjInfo> CameraComp::CreateShadowVPMatrix(const Vec3 lightDirection)
    {
        auto lightForward = -lightDirection;
        auto cameraPos = GetOwner()->transform->GetWorldPosition();
        
        constexpr float range = 30;
        float range2 = 20;
        auto distancePerTexel = range * 2 / static_cast<float>(GetRC()->mainLightShadowSize);
        // 计算阴影矩阵
        Vec3 forward, right, up;
        forward = lightForward;
        gram_schmidt_ortho(&forward.x, &Vec3::Up().x, &right.x, &up.x);
        auto shadowCameraToWorld = Matrix4x4(
            right.x, up.x, forward.x, 0,
            right.y, up.y, forward.y, 0,
            forward.x, forward.y, forward.z, 0,
            0, 0, 0, 1);
        auto view = shadowCameraToWorld.Inverse();
        // 希望以摄像机为中心，但是先把摄像机位置转到阴影空间，然后对齐每个纹素，避免阴影光栅化时闪烁
        auto cameraPositionVS = view * Vec4(cameraPos, 1);
        cameraPositionVS.x = std::floor(cameraPositionVS.x / distancePerTexel) * distancePerTexel;
        cameraPositionVS.y = std::floor(cameraPositionVS.y / distancePerTexel) * distancePerTexel;
        auto alignedCameraPositionWS = (shadowCameraToWorld * cameraPositionVS).ToVec3();
        // 得到对齐后的摄像机位置
        auto viewCenter = alignedCameraPositionWS + forward * range2;
        // 把阴影矩阵的中心设置为对齐后的摄像机位置
        shadowCameraToWorld[0][3] = viewCenter.x; // 第3列第0行
        shadowCameraToWorld[1][3] = viewCenter.y;
        shadowCameraToWorld[2][3] = viewCenter.z;
        view = shadowCameraToWorld.Inverse();

        auto proj = create_ortho_projection(range, -range, range, -range, 2 * range2, 0.05f);

        return ViewProjInfo::Create(view, proj, viewCenter);
    }
}
