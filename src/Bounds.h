#pragma once

#include "glm/glm.hpp"

namespace op
{
    class Bounds
    {
    public:
        glm::vec3 center = glm::vec3(0, 0, 0);
        glm::vec3 extents = glm::vec3(0.5, 0.5, 0.5);

        Bounds() = default;
        Bounds(const glm::vec3& center, const glm::vec3& extents);
    };
}
