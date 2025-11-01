#include "opengl_test.h"

#include <fstream>
#include <iostream>

#include "utils.h"

#include "game_framework.h"
#include "string_handle.h"
// #include "common/data_set.h"
#include "common/consumer_thread.h"
#include "common/elem_accessor_fixed.h"
#include "common/i_memory_block.h"
#include "common/managed_buffer.h"
#include "common/elem_accessor_var.h"
#include "common/thread_pool.h"
#include "common/thread_pool.h"
#include "job_system/job_scheduler.h"
#include "math/math_utils.h"
#include "math/matrix4x4.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/simd_math.h"

#include "spirv_cross/spirv_glsl.hpp"
#include "spirv_cross/spirv_msl.hpp"

using namespace op;

static void ReleaseStaticRes()
{
    
}

int main(int argc, char* argv[])
{
    // auto position = Vec3(0.0f, 0.0f, 0.0f);
    // auto forward = Vec3(0.0f, 0.0f, 1.0f);
    // Vec3 right, up;
    // gram_schmidt_ortho(&forward.x, &Vec3::Up().x, &right.x, &up.x);
    // auto localToWorld = Matrix4x4(
    //     right.x, up.x, forward.x, position.x,
    //     right.y, up.y, forward.y, position.y,
    //     right.z, up.z, forward.z, position.z,
    //     0.0f, 0.0f, 0.0f, 1.0f);
    // auto view = localToWorld.Inverse();
    // auto proj = create_ortho_projection(10, -10, 10, -10, 10, 1);
    // auto viewProj = proj * view;
    //
    // auto test0 = Vec3(0.0f, 0.0f, 1.0f);
    // auto test1 = Vec3(0.0f, 0.0f, 10.0f);
    // auto test2 = Vec3(10.0f, 0.0f, 10.0f);
    // auto test3 = Vec3(10.0f, 0.0f, 5.5f);
    //
    // auto result0 = viewProj * Vec4(test0, 1.0f);
    // auto result1 = viewProj * Vec4(test1, 1.0f);
    // auto result2 = viewProj * Vec4(test2, 1.0f);
    // auto result3 = viewProj * Vec4(test3, 1.0f);
    //
    // log_info("%s", result0.ToString().c_str());
    // log_info("%s", result1.ToString().c_str());
    // log_info("%s", result2.ToString().c_str());
    // log_info("%s", result3.ToString().c_str());
    //
    // return 0;
    
    auto gf = new GameFramework();
    try
    {
        if(!gf->Init())
        {
            return -1;
        }
        
        gf->GameLoop();
    
        delete gf;
    
        ReleaseStaticRes();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
    
    return 0;
}
