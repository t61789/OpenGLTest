#include "OpenGLTest.h"

#include <iostream>

#include "GameFramework.h"

int main(int argc, char* argv[])
{
    auto gf = std::make_unique<GameFramework>();
    try
    {
        if(!gf->Init())
        {
            return -1;
        }
        
        gf->GameLoop();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
    
    return 0;
}
