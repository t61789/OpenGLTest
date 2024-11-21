#pragma once
#include <string>

#include "Object.h"

class Scene
{
public:
    OBJECT_ID m_sceneRoot = UNDEFINED_RESOURCE;

    glm::vec3 m_lightDirection = glm::vec3(-1, -1, -1);
    
    Scene(const std::string& sceneJsonPath);

private:
    void LoadScene(const std::string& sceneJsonPath);
    void LoadSceneConfig(const nlohmann::json& configJson);
};
