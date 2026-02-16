#include <iostream>
#include "Nebula.h"



int main()
{
   

    // Create a window using the glfw library
    Nebula::Window window("Nebula", 800, 600);
    //making the window visible
    while (!window.closeWindow())
    {
        window.update();
    }

    // close the window and clean up resources


    


    return 0;
}