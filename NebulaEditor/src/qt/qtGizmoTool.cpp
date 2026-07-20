#include "qt/qtGizmoTool.h"

#include <cmath>

namespace Editor
{
  namespace
  {
    float angleAroundY(const ScreenRay &ray, const Nebula::Vec3 &origin)
    {
      Nebula::Vec3 hit{};
      if (!rayPlaneYIntersect(ray, origin.y, hit))
      {
        return 0.f;
      }
      return std::atan2(hit.z - origin.z, hit.x - origin.x);
    }
  }

  void QtGizmoTool::beginDrag(GizmoAxis axis, const Nebula::Vec3 &entityPos, float entityYaw,
                              const Nebula::Vec3 &entityScale, const ScreenRay &ray)
  {
    m_axis = axis;
    m_dragging = axis != GizmoAxis::None;
    m_startPos = entityPos;
    m_startYaw = entityYaw;
    m_startScale = entityScale;
    m_startAngle = angleAroundY(ray, entityPos);
  }

  bool QtGizmoTool::updateDrag(const ScreenRay &ray, Nebula::Vec3 &outPos, float &outYaw,
                               Nebula::Vec3 &outScale)
  {
    if (!m_dragging)
    {
      return false;
    }
    outPos = m_startPos;
    outYaw = m_startYaw;
    outScale = m_startScale;

    switch (m_mode)
    {
    case GizmoMode::Translate:
      return dragTranslateAxis(m_axis, ray, m_startPos, outPos);
    case GizmoMode::Rotate:
    {
      if (m_axis != GizmoAxis::Y && m_axis != GizmoAxis::None)
      {
        // Yaw-only engine transform: only Y rotation is authored.
        m_axis = GizmoAxis::Y;
      }
      const float angle = angleAroundY(ray, m_startPos);
      outYaw = m_startYaw + (angle - m_startAngle);
      return true;
    }
    case GizmoMode::Scale:
    {
      Nebula::Vec3 hit{};
      if (!rayPlaneYIntersect(ray, m_startPos.y, hit))
      {
        return false;
      }
      float delta = 0.f;
      if (m_axis == GizmoAxis::X)
      {
        delta = hit.x - m_startPos.x;
        outScale.x = std::max(0.05f, m_startScale.x + delta);
      }
      else if (m_axis == GizmoAxis::Y)
      {
        delta = hit.z - m_startPos.z; // approximate vertical scale from ground plane drag
        outScale.y = std::max(0.05f, m_startScale.y + delta);
      }
      else if (m_axis == GizmoAxis::Z)
      {
        delta = hit.z - m_startPos.z;
        outScale.z = std::max(0.05f, m_startScale.z + delta);
      }
      return true;
    }
    }
    return false;
  }

  void QtGizmoTool::endDrag()
  {
    m_dragging = false;
    m_axis = GizmoAxis::None;
  }

  GizmoAxis QtGizmoTool::hitTest(GizmoMode mode, const Nebula::Mat4 &viewProjection,
                                 const Nebula::Vec3 &origin, float screenX, float screenY,
                                 float viewportW, float viewportH) const
  {
    (void)mode;
    // Reuse translate axis picking for rotate rings / scale cubes (screen proximity).
    return pickTranslateAxis(viewProjection, origin, screenX, screenY, viewportW, viewportH);
  }

}
