#include "OpenGLTest.h"

#include <iostream>

#include "GameFramework.h"

int main(int argc, char* argv[])
{
    auto gf = std::make_unique<GameFramework>();
    try
    {
        if(!gf->init())
        {
            return -1;
        }
        
        gf->gameLoop();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
    
    return 0;
}
