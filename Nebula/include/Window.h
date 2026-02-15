#pragma once
#include <string_view>
#include <GLFW/glfw3.h>

namespace Nebula
{
    class Window
    {
    public:
        Window(std::string_view title, int width, int height);
        void create();
        void update();
        bool destroy();
        ~Window();
        
        
        
         
        
    

    private:
        GLFWwindow* m_window;
    };  
}