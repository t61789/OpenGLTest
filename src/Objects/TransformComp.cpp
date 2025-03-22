#include "TransformComp.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "Object.h"
#include "Utils.h"

namespace op
{
    using namespace std;
    using namespace glm;

    vec3 TransformComp::GetPosition()
    {
        return m_position.localVal;
    }

    void TransformComp::SetPosition(const vec3& pos)
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

    vec3 TransformComp::GetScale()
    {
        return m_scale.localVal;
    }

    void TransformComp::SetScale(const vec3& scale)
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

    quat TransformComp::GetRotation()
    {
        return m_rotation.localVal;
    }

    void TransformComp::SetRotation(const quat& rotation)
    {
        if (m_rotation.localVal == rotation)
        {
            return;
        }

        if (!m_matrix.worldDirty)
        {
            SetDirty(owner);
        }

        m_eulerAngles.localVal = eulerAngles(rotation);
        m_rotation.localVal = rotation;
        m_rotation.worldDirty = true;
    }

    vec3 TransformComp::GetEulerAngles()
    {
        return degrees(m_eulerAngles.localVal);
    }

    void TransformComp::SetEulerAngles(const vec3& ea)
    {
        auto e = radians(ea);
        if (m_eulerAngles.localVal == e)
        {
            return;
        }

        if (!m_matrix.worldDirty)
        {
            SetDirty(owner);
        }

        m_eulerAngles.localVal = e;
        m_rotation.localVal = quat(e);
        m_rotation.worldDirty = true;
    }

    mat4 TransformComp::GetLocalToWorld()
    {
        if (m_matrix.worldDirty)
        {
            UpdateMatrix();
        }

        return m_matrix.localVal;
    }

    vec3 TransformComp::Forward()
    {
        auto result = vec3(GetLocalToWorld()[2]);
        return {
            normalize(result)
        };
    }

    void TransformComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("position"))
        {
            SetPosition(Utils::ToVec3(objJson["position"]));
        }
    
        if(objJson.contains("rotation"))
        {
            SetEulerAngles(Utils::ToVec3(objJson["rotation"]));
        }
    
        if(objJson.contains("scale"))
        {
            SetScale(Utils::ToVec3(objJson["scale"]));
        }
    }

    void TransformComp::UpdateMatrix()
    {
        mat4 parentLocalToWorld;
        if (owner->parent)
        {
            // 如果parent还是dirty的话，GetLocalToWorld会继续往上递归地UpdateMatrix
            parentLocalToWorld = owner->parent->transform->GetLocalToWorld();
        }
        else
        {
            parentLocalToWorld = mat4(1);
        }

        auto objectMatrix = mat4(1);
        objectMatrix = translate(objectMatrix, m_position.localVal);
        objectMatrix = objectMatrix * mat4_cast(m_rotation.localVal);
        objectMatrix = scale(objectMatrix, m_scale.localVal);

        m_matrix.localVal = parentLocalToWorld * objectMatrix;
        m_matrix.worldVal = inverse(m_matrix.localVal);
        m_matrix.worldDirty = false;
    }

    /// 递归地将当前物体和它的子物体都标记为dirty
    void TransformComp::SetDirty(const Object* object)
    {
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
