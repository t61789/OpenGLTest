#include "OpenGLTest.h"

#include <iostream>

#include "BuiltInRes.h"
#include "GameFramework.h"

static void ReleaseStaticRes()
{
    Material::ReleaseStaticRes();
    BuiltInRes::ReleaseInstance();
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
