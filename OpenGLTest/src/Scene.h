#pragma once
#include <string>

#include "Object.h"

class Scene
{
public:
    Scene(const std::string& sceneJsonPath);
    OBJECT_ID m_sceneRoot;

private:
    void LoadScene(const std::string& sceneJsonPath);
};
