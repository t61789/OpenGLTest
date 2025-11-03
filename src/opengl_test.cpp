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
    // auto count = 1000000;
    // auto input = new uint32_t[count];
    // auto output = new uint32_t[count];
    // auto indices = new uint32_t[count];
    // for (auto i = 0; i < count; ++i)
    // {
    //     indices[i] = i;
    // }
    // for (auto i = 0; i < count; ++i)
    // {
    //     auto target = random_uint32(0, count - 1);
    //     auto tmp = indices[i];
    //     indices[i] = indices[target];
    //     indices[target] = tmp;
    // }
    //
    // auto sum0 = 0.0;
    // auto sum1 = 0.0;
    // for (auto i = 0; i < 100; ++i)
    // {
    //     auto start = std::chrono::high_resolution_clock::now();
    //     auto p = tbb::affinity_partitioner();
    //     oneapi::tbb::parallel_for(tbb::blocked_range(0, count), [&output, &input](const tbb::blocked_range<int>& j)
    //     {
    //         for (auto k = j.begin(); k != j.end(); ++k)
    //         {
    //             output[k] = std::sqrt(input[k] * 2.0f);
    //         }
    //     }, p);
    //     auto end = std::chrono::high_resolution_clock::now();
    //     sum0 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    //
    //     start = std::chrono::high_resolution_clock::now();
    //     auto outputPtr = output;
    //     auto lastPtr = output + count;
    //     while (outputPtr != lastPtr)
    //     {
    //         *outputPtr = std::sqrt(input[outputPtr - output] * 2.0f);
    //         outputPtr++;
    //     }
    //     end = std::chrono::high_resolution_clock::now();
    //     sum1 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // }
    //
    // std::cout << "std::sqrt: " << sum0 / 100 << "us" << std::endl;
    // std::cout << "std::sqrt: " << sum1 / 100 << "us" << std::endl;
    //
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
