#pragma once

#include "math/math.h"

#include "comp.h"
#include "event.h"

namespace op
{
    class TransformComp final : public Comp
    {
    public:
        Event<> dirtyEvent;

        void Awake() override;

        Vec3 GetWorldPosition();
        void SetWorldPosition(const Vec3& pos);
        Vec3 GetPosition();
        void SetPosition(const Vec3& pos);
        Vec3 GetScale();
        void SetScale(const Vec3& scale);
        Quaternion GetRotation(); // TODO 世界空间下的旋转
        void SetRotation(Quaternion& rotation);
        Vec3 GetEulerAngles();
        void SetEulerAngles(const Vec3& ea);
    
        void UpdateMatrix();
    
        const Matrix4x4& GetLocalToWorld();
        const Matrix4x4& GetWorldToLocal();

        void LoadFromJson(const nlohmann::json& objJson) override;

    private:
        template <typename T>
        class TransformCompProp
        {
        public:
            T localVal;
            T worldVal = T();
            bool needUpdateFromWorld = false;

            explicit TransformCompProp(T localVal) : localVal(localVal) {}
        };
    
        bool m_dirty = true;
        TransformCompProp<Vec3> m_position = TransformCompProp(Vec3());
        TransformCompProp<Quaternion> m_rotation = TransformCompProp(Quaternion());
        TransformCompProp<Vec3> m_eulerAngles = TransformCompProp(Vec3());
        TransformCompProp<Vec3> m_scale = TransformCompProp(Vec3(1.0f));
        TransformCompProp<Matrix4x4> m_matrix = TransformCompProp(Matrix4x4());

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
