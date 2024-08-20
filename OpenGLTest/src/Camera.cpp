#include "Camera.h"

#include <ext/matrix_transform.hpp>
#include <gtx/euler_angles.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 rotation) : Object(position, rotation)
{
}

void Camera::Update()
{
    glm::vec3 forward = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z) * glm::vec4(0, 0, -1.0f, 0.0f);
    // if(Window::KeyPressed(GLFW_KEY_W))
    // {
    //     position 
    // }
}
