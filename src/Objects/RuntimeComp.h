#pragma once
#include "Comp.h"

class RuntimeComp : public Comp
{
public:
    ~RuntimeComp() override;
    
    void Awake() override;

private:
    Object* m_groundGrid = nullptr;
};
