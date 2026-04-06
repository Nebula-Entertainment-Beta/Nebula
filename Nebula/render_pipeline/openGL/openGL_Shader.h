#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "../interface/shader.h"

namespace Nebula{
  class OpenGL_Shader : public Shader{
    public:
      OpenGL_Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
      ~OpenGL_Shader() override;
      void bind() const override;
      void unbind() const override;

      /// Returns nullptr if compile/link failed.
      static std::shared_ptr<Shader> create(const std::string& vertexSrc, const std::string& fragmentSrc);
    
    private:
      uint32_t m_rendererID =0;
  };
}