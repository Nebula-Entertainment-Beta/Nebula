#pragma once

#include <memory>
#include <string>

namespace Nebula{
  class Shader
  {
    public:
      virtual ~Shader() = default;
      virtual void bind() const = 0;
      virtual void unbind() const = 0;

      static std::shared_ptr<Shader> create(const std::string& vertexSrc, const std::string& fragmentSrc);
  };

}