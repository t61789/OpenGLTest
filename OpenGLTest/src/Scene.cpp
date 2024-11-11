#include "Scene.h"

#include <fstream>
#include <iostream>

#include "../lib/json.hpp"

Object* LoadObject(const nlohmann::json& objJson)
{
    Object* result;
    if(objJson.contains("type"))
    {
        auto type = objJson["type"].get<std::string>();
        if(type == "camera")
        {
            result = new Object();
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
    auto s = std::ifstream(sceneJsonPath);
    nlohmann::json json;
    s >> json;
    s.close();
    
    auto sceneRoot = new Object();
    sceneRoot->m_name = "Scene Root";
    
    AddTo(sceneRoot, json);
    
    m_sceneRoot = sceneRoot->m_id;
}
