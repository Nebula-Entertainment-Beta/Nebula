#pragma once
#include <memory>
#include <string>
#include "vertex_buffer.h"
#include "vertex_array.h"
#include "index_buffer.h"
#include "shader.h"
#include "texture.h"
#include "renderAPI.h"
namespace Nebula
{
  // OpenGL implementations — defined in openGL/openGL_factories.cpp
  std::shared_ptr<VertexBuffer> createOpenGLVertexBuffer(const void *data, size_t sizeBytes, BufferUsage usage, const VertexBufferLayout &layout);

  std::shared_ptr<VertexArray> createOpenGLVertexArray();

  std::shared_ptr<IndexBuffer> createOpenGLIndexBuffer(uint32_t *indices, uint32_t count);

  std::shared_ptr<Shader> createOpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);

  std::shared_ptr<Texture> createOpenGLTexture(int width, int height, const void *rgba);

  std::shared_ptr<Texture> createOpenGLTextureFromFile(const std::string &filepath);
  std::unique_ptr<RenderAPI> createOpenGLRendererAPI();
}