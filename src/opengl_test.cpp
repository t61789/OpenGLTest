#include "OpenGLTest.h"

#include <iostream>

#include "Utils.h"
#include "SharedObject.h"
#include "Material.h"
#include "GameFramework.h"

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
