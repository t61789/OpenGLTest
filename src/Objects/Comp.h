#pragma once
#include <json.hpp>

class Object;

class Comp
{
public:
    Object* owner = nullptr;

    virtual ~Comp() = default;

    virtual void Awake(){}

    virtual void Update(){}
    
    virtual void LoadFromJson(const nlohmann::json& objJson){}
};
