#include "Bounds.h"

namespace op
{
    Bounds::Bounds(const glm::vec3& center, const glm::vec3& extents)
    {
        this->center = center;
        this->extents = extents;
    }
}
