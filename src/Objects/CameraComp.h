#pragma once

#include "Object.h"

namespace op
{
    class CameraComp : public Comp
    {
    public:
        CameraComp();
        ~CameraComp() override;
        void Awake() override;
        void Update() override;
        float fov = 45.0f;
        float nearClip = 0.1f;
        float farClip = 30.0f;
        static CameraComp* GetMainCamera();
        void LoadFromJson(const nlohmann::json& objJson) override;
    
    private:
        glm::vec3 m_targetPosition = {};
        glm::vec3 m_targetRotation = {};
        static std::vector<CameraComp*> s_cameras;
    };
}
