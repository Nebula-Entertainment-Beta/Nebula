#include "renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Nebula
{
    class RenderAPI
    {
    public:
      virtual void init() = 0;
      virtual void Shutdown() = 0;
      virtual void drawIndexed(unsigned int count) = 0;
  
    };
    

}