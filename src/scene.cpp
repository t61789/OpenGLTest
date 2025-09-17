#include "scene.h"

#include <filesystem>

#include "game_resource.h"
#include "object.h"
#include "utils.h"
#include "nlohmann/json.hpp"
#include "objects/light_comp.h"
#include "objects/runtime_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    using namespace std;

    void Scene::LoadChildren(crsp<Object> parent, cr<nlohmann::json> children)
    {
        for(auto& elem : children)
        {
            auto obj = Object::CreateFromJson(elem);
            
            parent->AddChild(obj);
            
            if(elem.contains("children"))
            {
                LoadChildren(obj, elem["children"]);
            }
        }
    }

    sp<Scene> Scene::LoadScene(cr<StringHandle> sceneJsonPath)
    {
        {
            if (auto result = GetGR()->GetResource<Scene>(sceneJsonPath))
            {
                return result;
            }
        }

        auto scene = msp<Scene>();
        scene->m_objectIndices = mup<SceneObjectIndices>(scene);
        
        nlohmann::json json = Utils::LoadJson(sceneJsonPath);

        // 合并替换json
        auto p = filesystem::path(sceneJsonPath.CStr());
        auto coverSceneJsonPath = p.parent_path() / (p.stem().generic_string() + "_cover.json");
        if (exists(coverSceneJsonPath))
        {
            nlohmann::json coverSceneJson = Utils::LoadJson(coverSceneJsonPath.generic_string());
            Utils::MergeJson(json, coverSceneJson);
        }

        if(json.contains("config"))
        {
            scene->LoadSceneConfig(json["config"]);
        }

        if(json.contains("root"))
        {
            auto rootObj = Object::Create("Scene Root");
            scene->m_path = sceneJsonPath;
            scene->m_sceneRoot = rootObj;
            scene->m_sceneRoot->AddOrCreateComp<RuntimeComp>(RUNTIME_COMP);
            
            LoadChildren(rootObj, json["root"]);
        }

        GetGR()->RegisterResource(sceneJsonPath, scene);
        scene->m_path = sceneJsonPath;
        
        return scene;
    }

    void Scene::LoadSceneConfig(cr<nlohmann::json> configJson)
    {
        if(configJson.contains("ambientLightColorSky"))
        {
            ambientLightColorSky = configJson.at("ambientLightColorSky").get<Vec3>();
        }
        
        if(configJson.contains("ambientLightColorEquator"))
        {
            ambientLightColorEquator = configJson.at("ambientLightColorEquator").get<Vec3>();
        }
        
        if(configJson.contains("ambientLightColorGround"))
        {
            ambientLightColorGround = configJson.at("ambientLightColorGround").get<Vec3>();
        }

        if(configJson.contains("tonemappingExposureMultiplier"))
        {
            tonemappingExposureMultiplier = configJson.at("tonemappingExposureMultiplier").get<float>();
        }

        if (configJson.contains("fog_intensity"))
        {
            fogIntensity = configJson.at("fog_intensity").get<float>();
        }

        if (configJson.contains("fog_color"))
        {
            fogColor = configJson.at("fog_color").get<Vec3>();
        }
    }
}
