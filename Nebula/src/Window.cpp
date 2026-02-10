#include <iostream>
#include <string_view>
#include "Window.h"

namespace Nebula
{
    Window::Window(std::string_view title, int width, int height)
    {
        std::cout << "Creating Window with title " << title << std::endl;
        std::cout << "Width: " << width << " Height: " << height << std::endl;
    }

    Window::~Window()
    {
        // Destroy window resources here.
        std::cout << "Destroying Window..." << std::endl;
    }
}