#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/euler_angles.hpp>

#include "GameFramework.h"

Camera::Camera(glm::vec3 position, glm::vec3 rotation) : Object(position, rotation)
{
    _targetPosition = position;
    _targetRotation = rotation;
}

void Camera::Update()
{
    auto localToWorld = GetLocalToWorld();
    glm::vec3 forward = localToWorld * glm::vec4(0, 0, -1.0f, 0.0f);
    glm::vec3 right = localToWorld * glm::vec4(1.0f, 0, 0, 0.0f);

    float moveSpeed = 3;
    float rotateSpeed = 80;
    float damp = 0.07f;

    if(GameFramework::instance->KeyPressed(GLFW_KEY_W))
    {
        _targetPosition += forward * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_S))
    {
        _targetPosition += -forward * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_A))
    {
        _targetPosition += -right * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_D))
    {
        _targetPosition += right * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_E))
    {
        _targetPosition += glm::vec3(0, 1.0f, 0) * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }

    if(GameFramework::instance->KeyPressed(GLFW_KEY_Q))
    {
        _targetPosition += glm::vec3(0, -1.0f, 0) * GameFramework::instance->GetDeltaTime() * moveSpeed;
    }

    position = lerp(position, _targetPosition, damp);
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_UP))
    {
        _targetRotation.x += GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_DOWN))
    {
        _targetRotation.x += -GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_LEFT))
    {
        _targetRotation.y += GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }
    
    if(GameFramework::instance->KeyPressed(GLFW_KEY_RIGHT))
    {
        _targetRotation.y += -GameFramework::instance->GetDeltaTime() * rotateSpeed;
    }

    rotation = lerp(rotation, _targetRotation, damp);
}
