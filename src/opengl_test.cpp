#include "opengl_test.h"

#include <iostream>

#include "utils.h"
#include "shared_object.h"
#include "material.h"
#include "game_framework.h"
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
    // Vec4 v = Vec4(-1.0f, 0, -3.0f, 0);
    //
    // sign(&v.x, &v.x);
    //
    // Utils::Log(Info, "%s", v.ToString().c_str());
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
