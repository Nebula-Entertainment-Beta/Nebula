#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>


namespace Nebula
{
    class Renderer {

      public:
          void init();
          void render();
          void cleanup();
      private:
      
          GLuint m_VAO, m_VBO, m_EBO;
          

    };


}