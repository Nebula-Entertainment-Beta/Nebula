#pragma once

#include "graphicsContext.h"

class QOpenGLWidget;

namespace Editor
{

  class QtGraphicsContext final : public Nebula::graphicsContext
  {
  public:
    explicit QtGraphicsContext(QOpenGLWidget *widget);

    void makeCurrent() override;
    void swap() override;
    bool isValid() const override;
    void getFramebufferSize(int &width, int &height) const override;

  private:
    QOpenGLWidget *m_widget = nullptr;
  };

}
