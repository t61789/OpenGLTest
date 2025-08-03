#pragma once
#include "nlohmann/json.hpp"

namespace op
{
    class Object;

    class Comp
    {
    public:
        Object* owner = nullptr;

        bool IsStarted() const { return m_isStarted; }
        void SetIsStarted(const bool val) { m_isStarted = val; }

        void SetName(const std::string& name);
        const std::string& GetName() const { return m_name; }

        virtual ~Comp() = default;

        virtual void Awake(){}

        virtual void Start(){}

        virtual void OnDestroy(){}

        virtual void Update(){}
    
        virtual void LoadFromJson(const nlohmann::json& objJson){}

    private:
        bool m_isStarted = false;
        std::string m_name;
    };
}
