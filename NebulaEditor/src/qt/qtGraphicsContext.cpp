#include "qt/qtGraphicsContext.h"

#include <QOpenGLWidget>

namespace Editor
{

  QtGraphicsContext::QtGraphicsContext(QOpenGLWidget *widget) : m_widget(widget) {}

  void QtGraphicsContext::makeCurrent()
  {
    if (m_widget != nullptr)
    {
      m_widget->makeCurrent();
    }
  }

  void QtGraphicsContext::swap()
  {
    // QOpenGLWidget presents automatically at end of paintGL.
  }

  bool QtGraphicsContext::isValid() const
  {
    return m_widget != nullptr && m_widget->isValid();
  }

  void QtGraphicsContext::getFramebufferSize(int &width, int &height) const
  {
    if (m_widget == nullptr)
    {
      width = 0;
      height = 0;
      return;
    }
    const qreal dpr = m_widget->devicePixelRatioF();
    width = static_cast<int>(m_widget->width() * dpr);
    height = static_cast<int>(m_widget->height() * dpr);
  }

}
