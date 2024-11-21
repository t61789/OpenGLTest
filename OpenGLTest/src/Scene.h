#pragma once
#include <string>

#include "Object.h"

class Scene
{
public:
    OBJECT_ID sceneRoot = UNDEFINED_RESOURCE;

    glm::vec3 mainLightDirection = glm::vec3(1, 1, 1);
    glm::vec3 mainLightColor = glm::vec3(1, 1, 1);
    glm::vec3 ambientLightColor = glm::vec3(0, 0, 0);
    
    Scene(const std::string& sceneJsonPath);

private:
    void _loadScene(const std::string& sceneJsonPath);
    void _loadSceneConfig(const nlohmann::json& configJson);
};
