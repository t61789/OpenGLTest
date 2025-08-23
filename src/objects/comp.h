#pragma once
#include "string_handle.h"
#include "nlohmann/json.hpp"


namespace op
{
    class Scene;
    class Object;
    class GameFramework;

    class Comp
    {
    public:
        friend class Object;
        friend class GameFramework;
        friend class SceneObjectIndices;

        void SetEnable(bool enable);
        bool GetEnable() { return m_enable;}
        Object* GetOwner() const { return m_owner; }
        const StringHandle& GetName() const { return m_name; }

        virtual void Awake(){}
        virtual void Start(){}
        virtual void OnDestroy(){}
        virtual void OnEnable(){}
        virtual void OnDisable(){}
        virtual void Update(){}

        virtual ~Comp() = default;
    
        virtual void LoadFromJson(const nlohmann::json& objJson){}

    private:
        bool m_isStarted = false;
        bool m_enable = false;
        StringHandle m_name;
        Object* m_owner = nullptr;
        Scene* m_scene = nullptr;
        
        bool IsStarted() const { return m_isStarted;}
        void SetIsStarted(const bool val) { m_isStarted = val;}
        void SetName(const StringHandle& name) { m_name = name;}
        void SetScene(Scene* scene) { m_scene = scene;}
    };
}
