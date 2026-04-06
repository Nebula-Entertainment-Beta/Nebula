#include <iostream>
#include <glm/glm.hpp>
#include "Window.h"
#include "renderer.h"


int main()
{
    Nebula::Window window("Nebula", 800, 600);
    Nebula::Renderer::init();
    while (!window.shouldWindowClose())
    {
        Nebula::Renderer::setViewport(0, 0, 800, 600);
        Nebula::Renderer::clear( glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
        window.update();
    }
    Nebula::Renderer::Shutdown();

    return 0;
}
