#pragma once

#include "math/math.h"
#include "object.h"

namespace op
{
    class CameraComp final : public Comp
    {
    public:
        void Awake() override;
        void OnDestroy() override;
        void Start() override;
        void Update() override;
        float fov = 60.0f;
        float nearClip = 0.1f;
        float farClip = 30.0f;
        
        static CameraComp* GetMainCamera();
        
        void LoadFromJson(cr<nlohmann::json> objJson) override;
    
    private:
        Vec3 m_targetPosition = {};
        Vec3 m_targetRotation = {};

        float m_curSpeedAdd = 0;
        
        static vec<CameraComp*> m_cameras;
    };
}
