#include "scene.h"

#include <fstream>
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
        objectIndices = std::make_unique<SceneObjectIndices>(this);
    }

    Scene::~Scene()
    {
        if (sceneRoot)
        {
            DECREF(sceneRoot)
        }
    }

    void Scene::LoadChildren(Object* parent, const nlohmann::json& children)
    {
        for(auto elem : children)
        {
            auto obj = Object::CreateFromJson(elem);
            
            parent->AddChild(obj);
            
            if(elem.contains("children"))
            {
                LoadChildren(obj, elem["children"]);
            }
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

        auto scene = new Scene();
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
            scene->LoadSceneConfig(json["config"]);
        }

        if(json.contains("root"))
        {
            auto rootObj = Object::Create("Scene Root");
            rootObj->scene = scene;
            INCREF_BY(rootObj, scene)
            scene->sceneRoot = rootObj;
            scene->sceneRoot->AddOrCreateComp<RuntimeComp>(RUNTIME_COMP);
            
            LoadChildren(rootObj, json["root"]);
        }

        RegisterResource(sceneJsonPath, scene);
        
        return scene;
    }

    void Scene::OnObjectChildAdded(Object* parent, Object* child)
    {
        
    }

    void Scene::LoadSceneConfig(const nlohmann::json& configJson)
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
