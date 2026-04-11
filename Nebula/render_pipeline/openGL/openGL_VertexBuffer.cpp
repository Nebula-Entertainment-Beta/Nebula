/**
 * @file openGL_VertexBuffer.cpp
 * @brief Creates and uploads vertex data with `glBufferData`.
 */
#include "openGL_VertexBuffer.h"
#include <glad/glad.h>

namespace Nebula{

  static GLenum toGLBufferUsage(BufferUsage usage){
    switch (usage){
      case BufferUsage::Static: return GL_STATIC_DRAW;
      case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
      case BufferUsage::Stream: return GL_STREAM_DRAW;
    }
    return GL_STATIC_DRAW;
  }

  OpenGL_VertexBuffer::OpenGL_VertexBuffer(const void* data,size_t sizeBytes,BufferUsage usage,const VertexBufferLayout& layout):m_layout(layout)
  {
    glGenBuffers(1, &m_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeBytes),data,toGLBufferUsage(usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);


  }
  OpenGL_VertexBuffer::~OpenGL_VertexBuffer(){
    glDeleteBuffers(1, &m_rendererID);
  }

  void OpenGL_VertexBuffer:: bind() const{
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
  }

  void OpenGL_VertexBuffer:: unbind() const{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  std::shared_ptr<VertexBuffer> OpenGL_VertexBuffer::create(const void* data,size_t sizeBytes,BufferUsage usage,const VertexBufferLayout& layout) 
  {
    return std::make_shared<OpenGL_VertexBuffer>(data, sizeBytes, usage, layout);
  }



}
