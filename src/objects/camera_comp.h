#pragma once

#include "math/math.h"
#include "object.h"

namespace op
{
    class CameraComp : public Comp
    {
    public:
        void Awake() override;
        void OnDestroy() override;
        void Start() override;
        void Update() override;
        float fov = 45.0f;
        float nearClip = 0.1f;
        float farClip = 30.0f;
        static CameraComp* GetMainCamera();
        void LoadFromJson(const nlohmann::json& objJson) override;
    
    private:
        Vec3 m_targetPosition = {};
        Vec3 m_targetRotation = {};

        float m_curSpeedAdd = 0;
        
        static std::vector<CameraComp*> s_cameras;
    };
}
