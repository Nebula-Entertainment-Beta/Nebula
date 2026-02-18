#include <iostream>
#include "Window.h"

int main()
{
    Nebula::Window window("Nebula", 800, 600);

    while (!window.closeWindow())
    {
        window.update();
    }

    return 0;
}