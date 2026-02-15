#include <iostream>
#include "Nebula.h"
#include "header.h"
#include <GLFW/glfw3.h>
#include <Window.cpp>

int main()
{
    std::cout << "Hello, world!" << std::endl;
    std::cout << "The magic number is " << magicNumber << std::endl;

    // Create a window using the glfw library
    Nebula::Window window("Nebula", 800, 600);
    //making the window visible
    while (!window.destroy())
    {
        window.update();
    }

    // Destroy the window and clean up resources
    window.destroy();

    return 0;
}