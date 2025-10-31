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
    // auto count = 1000000;
    // auto data = new uint32_t[count];
    // auto result = new uint32_t[count];
    //
    // auto threadPoolNew = ThreadPoolNew(2);
    //
    // std::mutex mtx;
    // std::condition_variable cv;
    // auto taskCount = 0;
    //
    // double sum0 = 0;
    // double sum1 = 0;
    //
    // std::chrono::high_resolution_clock::time_point start, end;
    // for (int i = 0; i < 100; ++i)
    // {
    //     start = std::chrono::high_resolution_clock::now();
    //     for (uint32_t k = 0; k < count; ++k)
    //     {
    //         result[k] = std::sqrt(data[k] * data[k] + 1.0) * 2;
    //     }
    //     end = std::chrono::high_resolution_clock::now();
    //     sum0 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    //
    //     start = std::chrono::high_resolution_clock::now();
    //
    //     taskCount = 0;
    //     threadPoolNew.Run([&]
    //     {
    //         for (uint32_t k = 0; k < count / 2; ++k)
    //         {
    //             result[k] = std::sqrt(data[k] * data[k] + 1.0) * 2;
    //         }
    //     
    //         std::lock_guard lk(mtx);
    //         ++taskCount;
    //         if (taskCount == 2)
    //         {
    //             cv.notify_one();
    //         }
    //     });
    //     
    //     threadPoolNew.Run([&]
    //     {
    //         for (uint32_t k = count / 2; k < count; ++k)
    //         {
    //             result[k] = std::sqrt(data[k] * data[k] + 1.0) * 2;
    //         }
    //         
    //         std::lock_guard lk(mtx);
    //         ++taskCount;
    //         if (taskCount == 2)
    //         {
    //             cv.notify_one();
    //         }
    //     });
    //
    //     {
    //         std::unique_lock lk(mtx);
    //         cv.wait(lk);
    //     }
    //
    //     end = std::chrono::high_resolution_clock::now();
    //     sum1 += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // }
    //
    // std::cout << "sum0: " << sum0 / 100 << "\n";
    // std::cout << "sum1: " << sum1 / 100 << "\n";
    //
    // delete[] data;
    // delete[] result;
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
