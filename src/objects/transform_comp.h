#pragma once

#include "math/math.h"

#include "comp.h"
#include "event.h"

namespace op
{
    class TransformComp : public Comp
    {
    public:
        Event<> dirtyEvent;
        
        Vec3 GetPosition();
        void SetPosition(const Vec3& pos);
        Vec3 GetScale();
        void SetScale(const Vec3& scale);
        Quaternion GetRotation();
        void SetRotation(Quaternion& rotation);
        Vec3 GetEulerAngles();
        void SetEulerAngles(const Vec3& ea);
    
        const Matrix4x4& GetLocalToWorld();

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
        TransformCompProp<Vec3> m_position = TransformCompProp(Vec3());
        TransformCompProp<Quaternion> m_rotation = TransformCompProp(Quaternion());
        TransformCompProp<Vec3> m_eulerAngles = TransformCompProp(Vec3());
        TransformCompProp<Vec3> m_scale = TransformCompProp(Vec3(1.0f));
        TransformCompProp<Matrix4x4> m_matrix = TransformCompProp(Matrix4x4());
    
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
