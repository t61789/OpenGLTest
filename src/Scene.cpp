﻿#include "Scene.h"

#include <fstream>
#include <iostream>

#include "Utils.h"
#include "json.hpp"
#include "Objects/LightComp.h"

using namespace std;

Scene::Scene()
{
    m_objectChildAddedCallback = CreateCallback(this, &Scene::OnObjectChildAdded);
}

Scene::~Scene()
{
    if (sceneRoot)
    {
        DECREF(sceneRoot);
    }

    delete m_objectChildAddedCallback;
}

void Scene::LoadChildren(Object* parent, const nlohmann::json& children)
{
    for(auto elem : children)
    {
        auto obj = new Object();
        obj->LoadFromJson(elem);
        
        if(elem.contains("children"))
        {
            LoadChildren(obj, elem["children"]);
        }
        
        parent->AddChild(obj);
    }
}

vector<Object*>* Scene::GetAllObjects()
{
    auto result = new vector<Object*>();

    function<void(Object*)> fuc = [&](Object* obj)
    {
        if (!obj)
        {
            return;
        }

        result->push_back(obj);

        for(auto child : obj->children)
        {
            fuc(child);
        }
    };

    fuc(sceneRoot);

    return result;
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
    auto s = ifstream(Utils::GetRealAssetPath(sceneJsonPath));
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

void Scene::OnObjectChildAdded(Object* parent, Object* child)
{
    
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
