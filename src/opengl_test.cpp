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
#include "job_system/job_scheduler.h"
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
    // vec<uint32_t> data(1000000);
    // vec<uint32_t> result(data.size());
    // for (int i = 0; i < data.size(); ++i)
    // {
    //     data[i] = i;
    // }
    //
    // JobScheduler js;
    //
    // double sum0 = 0;
    // double sum1 = 0;
    //
    // std::chrono::high_resolution_clock::time_point start, end;
    // for (int i = 0; i < 100; ++i)
    // {
    //     start = std::chrono::high_resolution_clock::now();
    //     for (uint32_t k = 0; k < data.size(); ++k)
    //     {
    //         result[k] = data[k] * 2;
    //     }
    //     end = std::chrono::high_resolution_clock::now();
    //     sum0 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    //
    //     start = std::chrono::high_resolution_clock::now();
    //     auto id = js.Schedule(data.size(), [&data, &result](uint32_t start, uint32_t end)
    //     {
    //         for (uint32_t i = start; i < end; ++i)
    //         {
    //             result[i] = data[i] * 2;
    //         }
    //     });
    //
    //     js.Wait(id);
    //     end = std::chrono::high_resolution_clock::now();
    //     sum1 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // }
    //
    // std::cout << "sum0: " << sum0 / 100 << "\n";
    // std::cout << "sum1: " << sum1 / 100 << "\n";
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
