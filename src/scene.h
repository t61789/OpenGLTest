#pragma once

#include <string>

#include "math/math.h"
#include "nlohmann/json.hpp"

#include "event.h"
#include "scene_object_indices.h"
#include "shared_object.h"

namespace op
{
    class Object;

    class Scene : public SharedObject
    {
    public:
        Object* sceneRoot = nullptr;
        std::unique_ptr<SceneObjectIndices> objectIndices;

        Vec3 ambientLightColorSky = Vec3(0, 0, 0);
        Vec3 ambientLightColorEquator = Vec3(0, 0, 0);
        Vec3 ambientLightColorGround = Vec3(0, 0, 0);
        float tonemappingExposureMultiplier = 1.0f;
        float fogIntensity = 0.01f;
        Vec3 fogColor = Vec3(1.0f, 1.0f, 1.0f);

        void GetAllObjects(std::vector<Object*>& result);
        
        static Scene* LoadScene(const std::string& sceneJsonPath);

    private:
        Scene();
        ~Scene() override;

        EventHandler m_objectChildAddedHandler = 0;
        void OnObjectChildAdded(Object* parent, Object* child);

        void LoadSceneConfig(const nlohmann::json& configJson);
        
        static void LoadChildren(Object* parent, const nlohmann::json& children);
    };
}
