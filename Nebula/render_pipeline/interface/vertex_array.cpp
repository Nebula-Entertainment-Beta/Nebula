/**
 * @file vertex_array.cpp
 * @brief Factory for `VertexArray` based on `RenderAPI::getAPI()`.
 */
#include "vertex_array.h"
#include "../openGL/openGL_VertexArray.h"
#include "renderAPI.h"

namespace Nebula {

    std::shared_ptr<VertexArray> VertexArray::create() {
    
        switch (RenderAPI::getAPI())
        {
            
            case RendererAPIType::OpenGL:
                return std::make_shared<OpenGL_VertexArray>();    
            
            default:
                return nullptr;
            }
        }
   
}