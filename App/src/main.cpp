#include <iostream>
#include "greet.h"

int main()
{
    std::string name;
    std::cout << "Enter your name: ";
    std::cin >> name;

    Nebula::Greet(name);
}