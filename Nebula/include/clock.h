#pragma once

namespace Nebula
{

  class clock
  {
  public:
    clock();
    double nowSeconds() const;

  private:
    double m_startSeconds;
  };

}
