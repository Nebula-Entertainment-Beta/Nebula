#pragma once

#include "editor/editorPicking.h"
#include "ecs/entity.h"
#include "math_types.h"

namespace Editor
{

  enum class GizmoMode
  {
    Translate,
    Rotate,
    Scale
  };

  class QtGizmoTool
  {
  public:
    void setMode(GizmoMode mode) { m_mode = mode; }
    GizmoMode mode() const { return m_mode; }

    void beginDrag(GizmoAxis axis, const Nebula::Vec3 &entityPos, float entityYaw,
                   const Nebula::Vec3 &entityScale, const ScreenRay &ray);
    bool updateDrag(const ScreenRay &ray, Nebula::Vec3 &outPos, float &outYaw, Nebula::Vec3 &outScale);
    void endDrag();
    bool isDragging() const { return m_dragging; }
    GizmoAxis activeAxis() const { return m_axis; }

    GizmoAxis hitTest(GizmoMode mode, const Nebula::Mat4 &viewProjection, const Nebula::Vec3 &origin,
                      float screenX, float screenY, float viewportW, float viewportH,
                      float axisLength = 1.f) const;

  private:
    GizmoMode m_mode = GizmoMode::Translate;
    GizmoAxis m_axis = GizmoAxis::None;
    bool m_dragging = false;
    Nebula::Vec3 m_startPos{};
    float m_startYaw = 0.f;
    Nebula::Vec3 m_startScale{1.f, 1.f, 1.f};
    float m_startAngle = 0.f;
  };

}
