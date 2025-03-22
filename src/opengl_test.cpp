#include "opengl_test.h"

#include <iostream>

#include "utils.h"
#include "shared_object.h"
#include "material.h"
#include "game_framework.h"

using namespace op;

static void ReleaseStaticRes()
{
    Material::ReleaseStaticRes();
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

    if (!SharedObject::m_count.empty())
    {
        Utils::LogWarning("SharedObject未完全释放");
    }
    
    return 0;
}
