#pragma once

#include <string>

#include "glm/glm.hpp"
#include "nlohmann/json.hpp"

#include "event.h"
#include "shared_object.h"

namespace op
{
    class Object;

    class Scene : public SharedObject
    {
    public:
        Object* sceneRoot = nullptr;

        glm::vec3 ambientLightColorSky = glm::vec3(0, 0, 0);
        glm::vec3 ambientLightColorEquator = glm::vec3(0, 0, 0);
        glm::vec3 ambientLightColorGround = glm::vec3(0, 0, 0);
        float tonemappingExposureMultiplier = 1.0f;
        float fogIntensity = 0.01f;
        glm::vec3 fogColor = glm::vec3(1.0f, 1.0f, 1.0f);

        void GetAllObjects(std::vector<Object*>& result);
        
        static Scene* LoadScene(const std::string& sceneJsonPath);

    private:
        Scene();
        ~Scene() override;

        EventCallback<void, Object*, Object*> m_objectChildAddedCallback = nullptr;
        void OnObjectChildAdded(Object* parent, Object* child);

        void LoadSceneConfig(const nlohmann::json& configJson);
        
        static void LoadChildren(Object* parent, const nlohmann::json& children);
    };
}
