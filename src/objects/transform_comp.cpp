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

    Vec3 TransformComp::GetWorldPosition()
    {
        if (m_position.needUpdateFromWorld)
        {
            return m_position.worldVal;
        }
        
        UpdateMatrix();
        
        return m_position.worldVal;
    }

    void TransformComp::SetWorldPosition(const Vec3& pos)
    {
        if (m_position.needUpdateFromWorld && m_position.worldVal == pos)
        {
            return;
        }
        
        m_position.worldVal = pos;
        m_position.needUpdateFromWorld = true;

        SetDirty(owner);
    }

    Vec3 TransformComp::GetPosition()
    {
        if (m_position.needUpdateFromWorld)
        {
            UpdateMatrix();
        }
        
        return m_position.localVal;
    }

    void TransformComp::SetPosition(const Vec3& pos)
    {
        m_position.needUpdateFromWorld = false;
        
        if (m_position.localVal == pos)
        {
            return;
        }

        SetDirty(owner);

        m_position.localVal = pos;
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

        SetDirty(owner);

        m_scale.localVal = scale;
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

        SetDirty(owner);

        m_eulerAngles.localVal = rotation.ToEuler();
        m_rotation.localVal = rotation;
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

        SetDirty(owner);

        m_eulerAngles.localVal = ea;
        m_rotation.localVal = Quaternion::Euler(ea);
    }

    const Matrix4x4& TransformComp::GetLocalToWorld()
    {
        UpdateMatrix();

        return m_matrix.localVal;
    }
    
    const Matrix4x4& TransformComp::GetWorldToLocal()
    {
        UpdateMatrix();

        return m_matrix.worldVal;
    }

    void TransformComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("position"))
        {
            m_position.localVal = objJson.at("position").get<Vec3>();
        }
    
        if(objJson.contains("rotation"))
        {
            m_eulerAngles.localVal = objJson.at("rotation").get<Vec3>();
            m_rotation.localVal = Quaternion::Euler(m_eulerAngles.localVal);
        }
    
        if(objJson.contains("scale"))
        {
            m_scale.localVal = objJson.at("scale").get<Vec3>();
        }
    }

    void TransformComp::UpdateMatrix()
    {
        if (!m_dirty)
        {
            return;
        }
        m_dirty = false;
        
        Matrix4x4 parentLocalToWorld;
        if (owner->parent)
        {
            // 如果parent还是dirty的话，GetLocalToWorld会继续往上递归地UpdateMatrix
            parentLocalToWorld = owner->parent->transform->GetLocalToWorld();
            if (m_position.needUpdateFromWorld)
            {
                auto& parentWorldToLocal = owner->parent->transform->GetWorldToLocal();
                m_position.localVal = (parentWorldToLocal * Vec4(m_position.worldVal, 1.0f)).ToVec3();
                m_position.needUpdateFromWorld = false;
            }
        }
        else
        {
            parentLocalToWorld = Matrix4x4::Identity();
            if (m_position.needUpdateFromWorld)
            {
                m_position.localVal = m_position.worldVal;
                m_position.needUpdateFromWorld = false;
            }
        }

        auto objectMatrix = Matrix4x4::TRS(m_position.localVal, m_rotation.localVal, m_scale.localVal);

        m_matrix.localVal = parentLocalToWorld * objectMatrix;
        m_matrix.worldVal = m_matrix.localVal.Inverse();

        m_position.worldVal = {
            m_matrix.localVal[0][3],
            m_matrix.localVal[1][3],
            m_matrix.localVal[2][3],
        };
    }

    /// 递归地将当前物体和它的子物体都标记为dirty
    void TransformComp::SetDirty(const Object* object)
    {
        if (object->transform->m_dirty)
        {
            return;
        }
        
        object->transform->m_dirty = true;
        object->transform->dirtyEvent.Invoke();
    
        if (object->children.empty())
        {
            return;
        }

        for (auto child : object->children)
        {
            if (child->transform->m_dirty)
            {
                continue;
            }

            SetDirty(child);
        }
    }
}
