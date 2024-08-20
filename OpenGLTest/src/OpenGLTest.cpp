#include "OpenGLTest.h"

#include "GameFramework.h"

int main(int argc, char* argv[])
{
    auto gf = new GameFramework();
    try
    {
        if(!gf->Init())
        {
            return -1;
        }
        
        gf->StartGameLoop();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }

    delete gf;
    return 0;
}
