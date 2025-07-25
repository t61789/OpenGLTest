#include "bounds.h"

namespace op
{
    Bounds::Bounds(const Vec3& center, const Vec3& extents)
    {
        this->center = center;
        this->extents = extents;
    }
}
