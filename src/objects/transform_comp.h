#pragma once
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>

#include "comp.h"

namespace op
{
    class TransformComp : public Comp
    {
    public:
        glm::vec3 GetPosition();
        void SetPosition(const glm::vec3& pos);
        glm::vec3 GetScale();
        void SetScale(const glm::vec3& scale);
        glm::quat GetRotation();
        void SetRotation(const glm::quat& rotation);
        glm::vec3 GetEulerAngles();
        void SetEulerAngles(const glm::vec3& ea);
    
        glm::mat4 GetLocalToWorld();
        glm::vec3 Forward();

        void LoadFromJson(const nlohmann::json& objJson) override;

    private:
        template <typename T>
        class TransformCompProp
        {
        public:
            T localVal;
            T worldVal = T();
            bool worldDirty = true;

            explicit TransformCompProp(T localVal) : localVal(localVal) {}
        };
    
        bool m_localDirty = true;
        bool m_worldDirty = true;
        bool m_matrixDirty = true;
        TransformCompProp<glm::vec3> m_position = TransformCompProp<glm::vec3>(glm::vec3());
        TransformCompProp<glm::quat> m_rotation = TransformCompProp<glm::quat>(glm::quat());
        TransformCompProp<glm::vec3> m_eulerAngles = TransformCompProp<glm::vec3>(glm::vec3());
        TransformCompProp<glm::vec3> m_scale = TransformCompProp<glm::vec3>(glm::vec3(1.0f));
        TransformCompProp<glm::mat4> m_matrix = TransformCompProp<glm::mat4>(glm::mat4());
    
        void UpdateMatrix();

        template <typename T, typename ExtractFromMatrix>
        T GetWorldVal(TransformCompProp<T>& prop, ExtractFromMatrix extractFromMatrix)
        {
            if (!prop.worldDirty)
            {
                return prop.worldVal;
            }
        
            prop.worldVal = extractFromMatrix(GetLocalToWorld());
            prop.worldDirty = false;
            return prop.worldVal;
        }

        static void SetDirty(const Object* object);
    };
}
