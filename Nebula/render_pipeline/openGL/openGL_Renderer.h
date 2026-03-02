#include<iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "renderAPI.h"

namespace Nebula
{
    class OpenGL_Renderer : public RenderAPI
    {
    public:
        void init() override
        {
            std::cout << "Initializing OpenGL Renderer" << std::endl;
        }

        void drawIndexed(unsigned int count) override
        {
            std::cout << "Drawing " << count << " indexed vertices with OpenGL" << std::endl;
        }

        void Shutdown() override
        {
            std::cout << "Shutting down OpenGL Renderer" << std::endl;
        }

    };
}
