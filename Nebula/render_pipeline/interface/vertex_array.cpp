#include <Nebula/render_pipeline/interface/vertex_array.h>
#include "openGL_VertexArray.h" 
#include "renderAPI.h"



namespace Nebula {

    
    std::shared_ptr<VertexArray> VertexArray::create() {
    
        switch (RenderAPI::getAPI())
        {
            
            case RendererAPIType::OpenGL:
                return std:: make_shared<OpenGL_VertexArray>();    
            
            default:
                nullptr;
            }
        }
   
}