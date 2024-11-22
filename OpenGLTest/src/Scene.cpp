#include "Scene.h"

#include <fstream>
#include <iostream>

#include "Camera.h"
#include "Entity.h"
#include "Utils.h"
#include "../lib/json.hpp"

Object* LoadObject(const nlohmann::json& objJson)
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
    
    result->loadFromJson(objJson);
    return result;
}

void AddTo(Object* parent, const nlohmann::json& children)
{
    for(auto elem : children)
    {
        auto obj = LoadObject(elem);
        
        if(elem.contains("children"))
        {
            AddTo(obj, elem["children"]);
        }
        
        parent->addChild(obj->id);
    }
}

Scene::Scene(const std::string& sceneJsonPath)
{
    _loadScene(sceneJsonPath);
}

void Scene::_loadScene(const std::string& sceneJsonPath)
{
    auto s = std::ifstream(Utils::GetRealAssetPath(sceneJsonPath));
    nlohmann::json json;
    s >> json;
    s.close();

    if(json.contains("config"))
    {
        _loadSceneConfig(json["config"]);
    }

    if(json.contains("root"))
    {
        auto rootObj = new Object();
        rootObj->name = "Scene Root";
        
        AddTo(rootObj, json["root"]);

        sceneRoot = rootObj->id;
    }
}

void Scene::_loadSceneConfig(const nlohmann::json& configJson)
{
    if(configJson.contains("mainLightDirection"))
    {
        mainLightDirection = Utils::ToVec3(configJson["mainLightDirection"]);
    }
    
    if(configJson.contains("mainLightColor"))
    {
        mainLightColor = Utils::ToVec3(configJson["mainLightColor"]);
    }
    
    if(configJson.contains("ambientLightColor"))
    {
        ambientLightColor = Utils::ToVec3(configJson["ambientLightColor"]);
    }

    if(configJson.contains("tonemappingExposureMultiplier"))
    {
        tonemappingExposureMultiplier = configJson["tonemappingExposureMultiplier"].get<float>();
    }
}
