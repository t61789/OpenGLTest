#pragma once
#include "comp.h"

namespace op
{
    class RuntimeComp : public Comp
    {
    public:
        ~RuntimeComp() override;
    
        void Awake() override;

    private:
        Object* m_groundGrid = nullptr;
    };
}
