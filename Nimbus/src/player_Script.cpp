#include "player_Script.h"
#include "nimbus_config.h"
#include "scriptParams.h"

#include <cmath>

namespace Nimbus
{

  void PlayerScript::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(self);
    m_moveSpeed = m_params.readScriptParamFloat(sc.paramsJson, "moveSpeed", 3.f);
  }

  Nebula::Entity PlayerScript::GetCamera(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    (void)self;
    return ctx.scene.findByTag(Nimbus::kMainCameraTag);
  }

  void PlayerScript::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float dt)
  {
    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }

    const Nebula::Entity cameraEntity = GetCamera(ctx, self);
    if (!ctx.scene.isValidEntity(cameraEntity))
    {
      return;
    }
    auto &cameraComponent = ctx.scene.getCamera(cameraEntity);
    auto &transformComponent = ctx.scene.getTransform(self);

    if (ctx.input == nullptr)
    {
      return;
    }
    const Nebula::FrameInput &f = ctx.input->frame();

    Nebula::Vec3 moveDir{f.moveX, 0.0f, f.moveY};

    if (moveDir.x != 0.0f || moveDir.z != 0.0f)
    {
      const float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
      moveDir.x /= len;
      moveDir.z /= len;
    }

    const float yaw = cameraComponent.yaw;

    const Nebula::Vec3 forward{-std::sin(yaw), 0.0f, -std::cos(yaw)};
    const Nebula::Vec3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};

    Nebula::Vec3 velocity{
        forward.x * moveDir.z + right.x * moveDir.x,
        forward.y * moveDir.z + right.y * moveDir.x,
        forward.z * moveDir.z + right.z * moveDir.x};
    velocity = velocity * (m_moveSpeed * dt);
    Nebula::Vec3 cubePos = transformComponent.transform.getPosition();
    cubePos += velocity;
    transformComponent.transform.setPosition(cubePos);
  }

}
