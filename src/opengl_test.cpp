#include "opengl_test.h"

#include <iostream>

#include "utils.h"
#include "shared_object.h"
#include "material.h"
#include "game_framework.h"
#include "string_handle.h"
#include "math/matrix4x4.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/simd_math.h"

using namespace op;

static void ReleaseStaticRes()
{
    Material::ReleaseStaticRes();
}

int main(int argc, char* argv[])
{
    // auto s0 = StringHandle("test0");
    // auto s1 = StringHandle("test1");
    // auto s2 = StringHandle("test2");
    // auto s3 = StringHandle("test3");
    //
    // auto dict = std::unordered_map<size_t, Matrix4x4>();
    // dict[s0.GetHash()] = Matrix4x4::Identity();
    // dict[s1.GetHash()] = Matrix4x4::Identity();
    // dict[s2.GetHash()] = Matrix4x4::Identity();
    // dict[s3.GetHash()] = Matrix4x4::Identity();
    //
    // auto start = std::chrono::high_resolution_clock::now();
    // for (int i = 0; i < 10000; ++i)
    // {
    //     dict[s2.GetHash()] = Matrix4x4::Identity();
    // }
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // Utils::Log(Info, "calc mvp %d", duration.count());
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
    
    if (!SharedObject::m_count.empty())
    {
        Utils::LogWarning("SharedObject未完全释放");
    }
    
    return 0;
}
