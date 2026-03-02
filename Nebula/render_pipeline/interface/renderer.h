#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>


namespace Nebula
{
    class Renderer {

      public:
          static void init();
          static void Shutdown();
          static void BeginFrame();
          static void EndFrame();
          static void drawIndexed(unsigned int count);

      private:
            Renderer() = default;
        

          
          

    };


}