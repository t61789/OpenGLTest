#pragma once
#include <string>

#include "Object.h"

class Scene : SharedObject
{
public:
    Object* sceneRoot = nullptr;

    glm::vec3 mainLightDirection = glm::vec3(1, 1, 1);
    glm::vec3 mainLightColor = glm::vec3(1, 1, 1);
    glm::vec3 ambientLightColorSky = glm::vec3(0, 0, 0);
    glm::vec3 ambientLightColorEquator = glm::vec3(0, 0, 0);
    glm::vec3 ambientLightColorGround = glm::vec3(0, 0, 0);
    float tonemappingExposureMultiplier = 1.0f;
    
    Scene();
    ~Scene() override;
    
    static Scene* LoadScene(const std::string& sceneJsonPath);

private:
    void LoadSceneConfig(const nlohmann::json& configJson);
};
