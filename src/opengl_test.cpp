#include "opengl_test.h"

#include <iostream>
//
// #include "utils.h"
// #include "shared_object.h"
// #include "material.h"
// #include "game_framework.h"
#include "math/matrix4x4.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/simd_math.h"

// using namespace op;
//
// static void ReleaseStaticRes()
// {
//     Material::ReleaseStaticRes();
// }

int main(int argc, char* argv[])
{
    auto m = op::Matrix4x4(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16);

    m = m.Inverse();

    std::cout << m.ToString().c_str() << "\n";
    
    // auto gf = new GameFramework();
    // try
    // {
    //     if(!gf->Init())
    //     {
    //         return -1;
    //     }
    //     
    //     gf->GameLoop();
    //
    //     delete gf;
    //
    //     ReleaseStaticRes();
    // }
    // catch (std::exception& e)
    // {
    //     std::cout << e.what() << "\n";
    // }
    //
    // if (!SharedObject::m_count.empty())
    // {
    //     Utils::LogWarning("SharedObject未完全释放");
    // }
    //
    return 0;
}
