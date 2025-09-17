#pragma once
#include "const.h"
#include "nlohmann/json.hpp"


namespace op
{
    class Scene;
    class Object;
    class GameFramework;

    class Comp
    {
        friend class Object;
        friend class GameFramework;
        friend class SceneObjectIndices;
        friend class Scene;
        
    public:
        void SetEnable(bool enable);
        bool GetEnable() { return m_enable;}
        Object* GetOwner() const { return m_owner; }
        cr<StringHandle> GetName() const { return m_name; }

        virtual void Awake(){}
        virtual void Start(){}
        virtual void OnDestroy(){}
        virtual void OnEnable(){}
        virtual void OnDisable(){}
        virtual void Update(){}

        Comp() = default;
        virtual ~Comp() = default;
        Comp(const Comp& other) = delete;
        Comp(Comp&& other) noexcept = delete;
        Comp& operator=(const Comp& other) = delete;
        Comp& operator=(Comp&& other) noexcept = delete;

        virtual void LoadFromJson(cr<nlohmann::json> objJson) {}

    private:
        bool m_isStarted = false;
        bool m_enable = false;
        StringHandle m_name;
        Object* m_owner = nullptr;
        Scene* m_scene = nullptr;
        
        bool IsStarted() const { return m_isStarted;}
        void SetIsStarted(const bool val) { m_isStarted = val;}
        void SetName(cr<StringHandle> name) { m_name = name;}
    };
}
