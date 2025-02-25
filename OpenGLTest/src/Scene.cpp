#include "Scene.h"

#include <fstream>
#include <iostream>

#include "Camera.h"
#include "Entity.h"
#include "Utils.h"
#include "../lib/json.hpp"

Scene::Scene() = default;

Scene::~Scene()
{
    if (sceneRoot)
    {
        DECREF(sceneRoot);
    }
}

static Object* LoadObject(const nlohmann::json& objJson)
{
    Object* result;
    if(objJson.contains("type"))
    {
        auto type = objJson["type"].get<std::string>();
        if(type == "camera")
        {
            result = new Camera();
        }
        else if(type == "entity")
        {
            result = new Entity();
        }
        else
        {
            result = new Object();
        }
    }
    else
    {
        result = new Object();
    }
    
    result->LoadFromJson(objJson);
    return result;
}

void Scene::LoadChildren(Object* parent, const nlohmann::json& children)
{
    for(auto elem : children)
    {
        auto obj = LoadObject(elem);
        
        if(elem.contains("children"))
        {
            LoadChildren(obj, elem["children"]);
        }
        
        parent->AddChild(obj);
    }
}

Scene* Scene::LoadScene(const std::string& sceneJsonPath)
{
    {
        SharedObject* result;
        if(TryGetResource(sceneJsonPath, result))
        {
            return dynamic_cast<Scene*>(result);
        }
    }

    auto result = new Scene();
    auto s = std::ifstream(Utils::GetRealAssetPath(sceneJsonPath));
    nlohmann::json json;
    s >> json;
    s.close();

    if(json.contains("config"))
    {
        result->LoadSceneConfig(json["config"]);
    }

    if(json.contains("root"))
    {
        auto rootObj = new Object();
        rootObj->name = "Scene Root";
        
        LoadChildren(rootObj, json["root"]);

        result->sceneRoot = rootObj;
        INCREF_BY(rootObj, result);
    }

    RegisterResource(sceneJsonPath, result);
    return result;
}

void Scene::LoadSceneConfig(const nlohmann::json& configJson)
{
    if(configJson.contains("mainLightDirection"))
    {
        mainLightDirection = Utils::ToVec3(configJson["mainLightDirection"]);
    }
    
    if(configJson.contains("mainLightColor"))
    {
        mainLightColor = Utils::ToVec3(configJson["mainLightColor"]);
    }
    
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
}
