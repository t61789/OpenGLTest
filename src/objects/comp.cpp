#include "comp.h"

#include "object.h"

namespace op
{
    void Comp::Destroy()
    {
        SetEnable(false);
        OnDestroy();
    }

    void Comp::SetEnable(const bool enable)
    {
        if (enable && !m_enable)
        {
            m_enable = true;

            UpdateRealEnable();
        }
        else if (!enable && m_enable)
        {
            m_enable = false;

            UpdateRealEnable();
        }
    }

    void Comp::UpdateRealEnable()
    {
        auto preRealEnable = m_realEnable;
        m_realEnable = m_enable && GetOwner()->IsEnable();

        if (m_realEnable != preRealEnable)
        {
            if (m_realEnable)
            {
                OnEnable();
            }
            else
            {
                OnDisable();
            }
        }
    }
}
