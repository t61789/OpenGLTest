#include "comp.h"

namespace op
{
    void Comp::SetEnable(const bool enable)
    {
        if (enable && !m_enable)
        {
            m_enable = true;
            OnEnable();
        }
        else if (!enable && m_enable)
        {
            m_enable = false;
            OnDisable();
        }
    }
}
