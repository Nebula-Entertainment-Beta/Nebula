#include <iostream>
#include "Nebula.h"
#include "header.h"
#include <GLFW/glfw3.h>
#include <Window.cpp>

int main()
{
    std::cout << "Hello, world!" << std::endl;
    std::cout << "The magic number is " << magicNumber << std::endl;

    Nebula::Window window("Nebula", 800, 600);
    
    while (true)
    {
        window.update();
    }

    

    return 0;
}