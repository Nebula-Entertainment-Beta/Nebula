#include <iostream>
#include <string_view>

namespace Nebula
{
    void Greet(std::string_view name)
    {
        std::cout << "Hello, " << name << std::endl;
    }
}