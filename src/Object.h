#pragma once
#include <string>
#include <vector>

#include "json.hpp"
#include "glm/glm.hpp"

#include "Event.h"
#include "SharedObject.h"
#include "Objects/Comp.h"

class TransformComp;

class Object : public SharedObject
{
public:
    Object();
    ~Object() override;
    
    bool enabled = true;
    
    std::string name = "Unnamed object";

    TransformComp* transform = nullptr;
    Object* parent = nullptr;
    std::vector<Object*> children;
    typedef std::function<void(Object* parent, Object* child)> ChildAddedCallback;
    Event<void(Object* parent, Object* child)> childAddedEvent;

    virtual void LoadFromJson(const nlohmann::json& objJson);

    void AddChild(Object* child);
    void RemoveChild(Object* child);
    std::string GetPathInScene() const;

    bool HasComp(const std::string& compName);
    Comp* GetComp(const std::string& compName);
    template <typename T>
    T* GetComp(const std::string& compName)
    {
        return dynamic_cast<T*>(GetComp(compName));
    }
    std::vector<Comp*> GetComps();
    Comp* AddComp(const std::string& compName);

private:
    std::unordered_map<std::string, Comp*> m_comps;

    static std::function<Comp*()> GetConstructor(const std::string& name);
};
