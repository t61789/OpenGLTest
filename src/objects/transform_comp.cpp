#include "transform_comp.h"

#include "object.h"
#include "utils.h"

namespace op
{
    using namespace std;

    void TransformComp::Awake()
    {
        owner->transform = this;
    }

    Vec3 TransformComp::GetPosition()
    {
        return m_position.localVal;
    }

    void TransformComp::SetPosition(const Vec3& pos)
    {
        if (m_position.localVal == pos)
        {
            return;
        }

        if (!m_matrix.worldDirty)
        {
            SetDirty(owner);
        }

        m_position.localVal = pos;
        m_position.worldDirty = true;
    }

    Vec3 TransformComp::GetScale()
    {
        return m_scale.localVal;
    }

    void TransformComp::SetScale(const Vec3& scale)
    {
        if (m_scale.localVal == scale)
        {
            return;
        }

        if (!m_matrix.worldDirty)
        {
            SetDirty(owner);
        }

        m_scale.localVal = scale;
        m_scale.worldDirty = true;
    }

    Quaternion TransformComp::GetRotation()
    {
        return m_rotation.localVal;
    }

    void TransformComp::SetRotation(Quaternion& rotation)
    {
        if (m_rotation.localVal == rotation)
        {
            return;
        }

        if (!m_matrix.worldDirty)
        {
            SetDirty(owner);
        }

        m_eulerAngles.localVal = rotation.ToEuler();
        m_rotation.localVal = rotation;
        m_rotation.worldDirty = true;
    }

    Vec3 TransformComp::GetEulerAngles()
    {
        return m_eulerAngles.localVal;
    }

    void TransformComp::SetEulerAngles(const Vec3& ea)
    {
        if (m_eulerAngles.localVal == ea)
        {
            return;
        }

        if (!m_matrix.worldDirty)
        {
            SetDirty(owner);
        }

        m_eulerAngles.localVal = ea;
        m_rotation.localVal = Quaternion::Euler(ea);
        m_rotation.worldDirty = true;
    }

    const Matrix4x4& TransformComp::GetLocalToWorld()
    {
        if (m_matrix.worldDirty)
        {
            UpdateMatrix();
            m_matrix.worldDirty = false;
        }

        return m_matrix.localVal;
    }

    void TransformComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("position"))
        {
            SetPosition(objJson.at("position").get<Vec3>());
        }
    
        if(objJson.contains("rotation"))
        {
            SetEulerAngles(objJson.at("rotation").get<Vec3>());
        }
    
        if(objJson.contains("scale"))
        {
            SetScale(objJson.at("scale").get<Vec3>());
        }
    }

    void TransformComp::UpdateMatrix()
    {
        Matrix4x4 parentLocalToWorld;
        if (owner->parent)
        {
            // 如果parent还是dirty的话，GetLocalToWorld会继续往上递归地UpdateMatrix
            parentLocalToWorld = owner->parent->transform->GetLocalToWorld();
        }
        else
        {
            parentLocalToWorld = Matrix4x4::Identity();
        }

        auto objectMatrix = Matrix4x4::TRS(m_position.localVal, m_rotation.localVal, m_scale.localVal);

        m_matrix.localVal = parentLocalToWorld * objectMatrix;
        m_matrix.worldVal = m_matrix.localVal.Inverse();
    }

    /// 递归地将当前物体和它的子物体都标记为dirty
    void TransformComp::SetDirty(const Object* object)
    {
        if (!object->transform->m_matrix.worldDirty)
        {
            object->transform->dirtyEvent.Invoke();
        }
        object->transform->m_matrix.worldDirty = true;
    
        if (object->children.empty())
        {
            return;
        }

        for (auto child : object->children)
        {
            if (child->transform->m_matrix.worldDirty)
            {
                continue;
            }

            SetDirty(child);
        }
    }
}
