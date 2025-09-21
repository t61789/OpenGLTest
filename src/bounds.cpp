#include "bounds.h"

namespace op
{
    Bounds::Bounds(const Vec3& center, const Vec3& extents)
    {
        this->center = center;
        this->extents = extents;
    }

    Bounds Bounds::ToWorld(cr<Matrix4x4> m) const
    {
        auto centerWS = Vec3(m * Vec4(center, 1));
        Vec3 extentsWS = {
            dot(abs(Vec3(m.GetRow(0))), extents),
            dot(abs(Vec3(m.GetRow(1))), extents),
            dot(abs(Vec3(m.GetRow(2))), extents)
        };

        return { centerWS, extentsWS };
    }
}
