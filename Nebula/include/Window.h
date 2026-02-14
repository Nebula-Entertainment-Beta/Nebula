#pragma once
#include <string_view>

namespace Nebula
{
    class Window
    {
    public:
        Window(std::string_view title, int width, int height);

        ~Window();
    };
    
       
}