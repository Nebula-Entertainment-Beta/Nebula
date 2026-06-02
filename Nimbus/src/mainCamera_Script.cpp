#include "mainCamera_Script.h"
#include "scriptParams.h"

#include <cmath>

namespace Nimbus
{

  void MainCameraScript::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(self);
    m_lookSensitivity = m_params.readScriptParamFloat(sc.paramsJson, "lookSensitivity", 0.0035f);
    m_zoomSpeed = m_params.readScriptParamFloat(sc.paramsJson, "zoomSpeed", 0.6f);
    m_pitchMin = m_params.readScriptParamFloat(sc.paramsJson, "pitchMin", -1.2f);
    m_pitchMax = m_params.readScriptParamFloat(sc.paramsJson, "pitchMax", 0.65f);
    /*m_minDistance = m_params.readScriptParamFloat(sc.paramsJson, "minDistance", 1.5f);
    m_maxDistance = m_params.readScriptParamFloat(sc.paramsJson, "maxDistance", 24.f); */
  }

  void MainCameraScript::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float dt)
  {
    (void)dt;
    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }

    auto &cameraComponent = ctx.scene.getCamera(self);

    if (ctx.input == nullptr)
    {
      return;
    }
    const Nebula::FrameInput &f = ctx.input->frame();

    const float turnX = f.lookX * m_lookSensitivity;
    const float turnY = f.lookY * m_lookSensitivity;
    cameraComponent.yaw -= turnX;
    cameraComponent.pitch = std::clamp(cameraComponent.pitch - turnY, m_pitchMin, m_pitchMax);

    /*float dist = cameraComponent.distance;
    dist -= f.zoomY * m_zoomSpeed;
    cameraComponent.distance = std::clamp(dist, m_minDistance, m_maxDistance);*/
  }

}
