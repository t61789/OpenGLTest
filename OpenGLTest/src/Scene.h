#pragma once
#include <string>

#include "Object.h"

class Scene
{
public:
    OBJECT_ID sceneRoot = UNDEFINED_RESOURCE;

    glm::vec3 lightDirection = glm::vec3(-1, -1, -1);
    
    Scene(const std::string& sceneJsonPath);

private:
    void _loadScene(const std::string& sceneJsonPath);
    void _loadSceneConfig(const nlohmann::json& configJson);
};
