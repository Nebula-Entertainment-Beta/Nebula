#pragma once

namespace Nebula
{
  class graphicsContext
  {
  public:
    virtual ~graphicsContext() = default;
    virtual void makeCurrent() = 0;
    virtual void swap() = 0;
    virtual bool isValid() const = 0;
    virtual void getFramebufferSize(int &width, int &height) const = 0;
  };
}