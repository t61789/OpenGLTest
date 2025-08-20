#include "opengl_test.h"

#include <fstream>
#include <iostream>

#include "utils.h"
#include "shared_object.h"

#include "game_framework.h"
#include "string_handle.h"
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

// static float GetFloatRandom()
// {
//     return static_cast<float>(std::rand()) / RAND_MAX;
// }
//
// static std::vector<float> GetRandomFloats()
// {
//     std::vector<float> result;
//     for (size_t i = 0; i < 64; i++)
//     {
//         float f = GetFloatRandom();
//         result.push_back(f);
//     }
//
//     return result;
// }
//
// static int GetRandCount()
// {
//     auto r = GetFloatRandom();
//     r *= 4;
//     if (r > 3)
//     {
//         return GetFloatRandom() * 36 + 36;
//     }
//     else if (r > 2)
//     {
//         return 16;
//     }
//     else if (r > 1)
//     {
//         return 4;
//     }
//     return 1;
// }
//
// struct CommonBuffer
// {
//     int index = -1;
//     float data;
// };

int main(int argc, char* argv[])
{
    // // std::unordered_map<int, CommonBuffer> commonBuffer;
    // // for (int i = 0; i < 16; ++i)
    // // {
    // //     commonBuffer[i] = { i, GetFloatRandom() };
    // // }
    // std::vector<CommonBuffer> commonBuffer;
    // for (int i = 0; i < 16; ++i)
    // {
    //     commonBuffer.push_back({i, GetFloatRandom()});
    //     // commonBuffer[i] = { i, GetFloatRandom() };
    // }
    //
    // auto ended = commonBuffer.end();
    // size_t sum = 0;
    // auto finded = 0;
    // for (int j = 0; j < 10000; ++j)
    // {
    //     auto start = std::chrono::high_resolution_clock::now();
    //     for (int i = 0; i < 10000; ++i)
    //     {
    //         // auto it = commonBuffer.find(-i);
    //         // if (it != ended)
    //         // {
    //         //     finded ++;
    //         // }
    //         auto ff = find(commonBuffer, &CommonBuffer::index, -i);
    //         if (ff)
    //         {
    //             finded ++;
    //         }
    //     }
    //     auto end = std::chrono::high_resolution_clock::now();
    //     sum += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    // }
    //
    // std::cout << "Time taken: " << sum / 10000.0f << "ms\n";
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
    
    if (!SharedObject::m_count.empty())
    {
        log_warning("SharedObject未完全释放");
    }
    
    return 0;
}
