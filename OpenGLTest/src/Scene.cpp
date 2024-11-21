#include "Scene.h"

#include <fstream>

#include "../lib/json.hpp"

glm::vec3 ToVec3(nlohmann::json arr)
{
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>()
    };
}

void AddTo(Object* parent, const nlohmann::json& children)
{
    for(auto elem : children)
    {
        auto obj = new Object();
        
        if(elem.contains("name"))
        {
            obj->m_name = elem["name"].get<std::string>();
        }
        if(elem.contains("position"))
        {
            obj->m_position = ToVec3(elem["position"]);
        }
        if(elem.contains("rotation"))
        {
            obj->m_position = ToVec3(elem["rotation"]);
        }
        if(elem.contains("scale"))
        {
            obj->m_position = ToVec3(elem["scale"]);
        }
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

    delete json;

    m_sceneRoot = sceneRoot->m_id;
}
