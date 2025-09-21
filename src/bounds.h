#pragma once

#include "math/math.h"

namespace op
{
    class Bounds
    {
    public:
        Vec3 center = Vec3(0, 0, 0);
        Vec3 extents = Vec3(0.5, 0.5, 0.5);

        Bounds() = default;
        Bounds(const Vec3& center, const Vec3& extents);

        Bounds ToWorld(cr<Matrix4x4> m) const;

        template <typename Archive>
        void serialize(Archive& archive, unsigned int version)
        {
            archive & center.x & center.y & center.z;
            archive & extents.x & extents.y & extents.z;
        }
    };
}
