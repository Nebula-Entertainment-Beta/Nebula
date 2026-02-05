#include <iostream>
#include <Nebula.h>
#include "header.h"

int main()
{
    std::cout << "Hello, world!" << std::endl;
    std::cout << "The magic number is " << magicNumber << std::endl;
    Nebula::Greet("John Doe");
}