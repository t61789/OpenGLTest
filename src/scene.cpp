#include "scene.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "utils.h"
#include "nlohmann/json.hpp"
#include "objects/light_comp.h"
#include "objects/runtime_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    using namespace std;

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        if (sceneRoot)
        {
            DECREF(sceneRoot);
        }
    }

    void Scene::LoadChildren(Object* parent, const nlohmann::json& children)
    {
        for(auto elem : children)
        {
            auto obj = Object::CreateFromJson(elem);
            
            if(elem.contains("children"))
            {
                LoadChildren(obj, elem["children"]);
            }
            
            parent->AddChild(obj);
        }
    }

    void Scene::GetAllObjects(vector<Object*>& result)
    {
        result.clear();
        function<void(Object*)> fuc = [&](Object* obj)
        {
            if (!obj)
            {
                return;
            }

            result.push_back(obj);

            for(auto child : obj->children)
            {
                fuc(child);
            }
        };

        fuc(sceneRoot);
    }

    Scene* Scene::LoadScene(const string& sceneJsonPath)
    {
        {
            SharedObject* result;
            if(TryGetResource(sceneJsonPath, result))
            {
                return dynamic_cast<Scene*>(result);
            }
        }

        auto result = new Scene();
        nlohmann::json json = Utils::LoadJson(sceneJsonPath);

        // 合并替换json
        auto p = filesystem::path(sceneJsonPath);
        auto coverSceneJsonPath = p.parent_path() / (p.stem().generic_string() + "_cover.json");
        if (exists(coverSceneJsonPath))
        {
            nlohmann::json coverSceneJson = Utils::LoadJson(coverSceneJsonPath.generic_string());
            Utils::MergeJson(json, coverSceneJson);
        }

        if(json.contains("config"))
        {
            result->LoadSceneConfig(json["config"]);
        }

        if(json.contains("root"))
        {
            auto rootObj = Object::Create("Scene Root");
            
            LoadChildren(rootObj, json["root"]);

            result->sceneRoot = rootObj;
            INCREF_BY(rootObj, result);

            result->sceneRoot->AddOrCreateComp<RuntimeComp>("RuntimeComp");
        }

        RegisterResource(sceneJsonPath, result);
        
        return result;
    }

    void Scene::OnObjectChildAdded(Object* parent, Object* child)
    {
        
    }

    void Scene::LoadSceneConfig(const nlohmann::json& configJson)
    {
        if(configJson.contains("ambientLightColorSky"))
        {
            ambientLightColorSky = Utils::ToVec3(configJson["ambientLightColorSky"]);
        }
        
        if(configJson.contains("ambientLightColorEquator"))
        {
            ambientLightColorEquator = Utils::ToVec3(configJson["ambientLightColorEquator"]);
        }
        
        if(configJson.contains("ambientLightColorGround"))
        {
            ambientLightColorGround = Utils::ToVec3(configJson["ambientLightColorGround"]);
        }

        if(configJson.contains("tonemappingExposureMultiplier"))
        {
            tonemappingExposureMultiplier = configJson["tonemappingExposureMultiplier"].get<float>();
        }

        if (configJson.contains("fog_intensity"))
        {
            fogIntensity = configJson["fog_intensity"].get<float>();
        }

        if (configJson.contains("fog_color"))
        {
            fogColor = Utils::ToVec3(configJson["fog_color"]);
        }
    }
}
