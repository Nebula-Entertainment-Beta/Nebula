#pragma once
#include <string_view>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// This class is responsible for creating and managing the application window using GLFW. 
//It provides methods to update the window, check if it should close, 
//and clean up resources when the window is destroyed.
namespace Nebula
{
    class Window
    {
    public:
        Window(std::string_view title, int width, int height);
        void update();
        bool shouldWindowClose();
        ~Window();
    private:
        GLFWwindow* m_window;
    };  
}
