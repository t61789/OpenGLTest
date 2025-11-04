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
#include "oneapi/tbb.h"

#include "spirv_cross/spirv_glsl.hpp"
#include "spirv_cross/spirv_msl.hpp"

using namespace op;

static void ReleaseStaticRes()
{
    
}

int main(int argc, char* argv[])
{
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
