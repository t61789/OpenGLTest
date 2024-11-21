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
    
    result->LoadFromJson(objJson);
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
        
        parent->AddChild(obj->m_id);
    }
}

Scene::Scene(const std::string& sceneJsonPath)
{
    LoadScene(sceneJsonPath);
}

void Scene::LoadScene(const std::string& sceneJsonPath)
{
    auto s = std::ifstream(Utils::GetRealAssetPath(sceneJsonPath));
    nlohmann::json json;
    s >> json;
    s.close();

    if(json.contains("config"))
    {
        LoadSceneConfig(json["config"]);
    }

    if(json.contains("root"))
    {
        auto sceneRoot = new Object();
        sceneRoot->m_name = "Scene Root";
        
        AddTo(sceneRoot, json["root"]);

        m_sceneRoot = sceneRoot->m_id;
    }
}

void Scene::LoadSceneConfig(const nlohmann::json& configJson)
{
    if(configJson.contains("lightDirection"))
    {
        m_lightDirection = Utils::ToVec3(configJson["lightDirection"]);
    }
}
