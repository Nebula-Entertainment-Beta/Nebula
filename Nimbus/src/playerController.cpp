#include "playerController.h"

#include "frameCommands.h"
#include "input_Actions.h"

#include <cmath>
#include <iostream>

namespace Nimbus
{

  void runPlayerController(Nebula::World &world,
                           Nebula::Entity cubeEntity,
                           Nebula::Entity cameraEntity,
                           bool &showInputDebug,
                           float &debugPrintTimer,
                           float dt)
  {
    Nebula::Scene &scene = world.scene();
    if (!scene.isValidEntity(cubeEntity) || !scene.isValidEntity(cameraEntity))
    {
      return;
    }
    if (!scene.hasComponent<Nebula::TransformComponent>(cubeEntity) ||
        !scene.hasComponent<Nebula::CameraComponent>(cameraEntity))
    {
      return;
    }

    const Nebula::FrameInput &f = world.frameInput();
    auto &cubeTransform = scene.getComponent<Nebula::TransformComponent>(cubeEntity);
    auto &cameraComponent = scene.getComponent<Nebula::CameraComponent>(cameraEntity);
    Nebula::ActionMapping &mapping = world.actions();

    if (f.toggleInputDebug)
    {
      showInputDebug = !showInputDebug;
    }

    const float lookSensitivity = 0.0035f;
    const float turnX = f.lookX * lookSensitivity;
    const float turnY = f.lookY * lookSensitivity;
    cameraComponent.yaw -= turnX;
    cameraComponent.pitch = std::clamp(cameraComponent.pitch - turnY, -1.2f, 0.65f);

    Nebula::Vec3 moveDir{f.moveX, 0.0f, f.moveY};
    if (moveDir.x != 0.0f || moveDir.z != 0.0f)
    {
      const float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
      moveDir.x /= len;
      moveDir.z /= len;
    }

    const float yaw = cameraComponent.yaw;
    const Nebula::Vec3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
    const Nebula::Vec3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
    Nebula::Vec3 velocity{
        forward.x * moveDir.z + right.x * moveDir.x,
        forward.y * moveDir.z + right.y * moveDir.x,
        forward.z * moveDir.z + right.z * moveDir.x};
    velocity = velocity * (3.5f * dt);
    Nebula::Vec3 cubePos = cubeTransform.transform.getPosition();
    cubePos += velocity;
    cubeTransform.transform.setPosition(cubePos);

    float dist = cameraComponent.distance;
    dist -= f.zoomY * 0.6f;
    cameraComponent.distance = std::clamp(dist, 1.5f, 24.0f);

    if (showInputDebug)
    {
      debugPrintTimer += dt;
      if (debugPrintTimer >= 0.25f)
      {
        debugPrintTimer = 0.0f;
        float sensX = 0.0f;
        float sensY = 0.0f;
        float zoomSens = 0.0f;
        bool invertY = false;
        mapping.getCameraSensitivity(sensX, sensY, zoomSens, invertY);
        std::cout
            << "[Mapping] MoveX=" << f.moveX
            << " MoveY=" << f.moveY
            << " LookX=" << f.lookX
            << " LookY=" << f.lookY
            << " Scroll=" << f.zoomY
            << " Sens=(" << sensX << ", " << sensY << ")"
            << " ZoomSens=" << zoomSens
            << " InvertY=" << invertY
            << '\n';
      }
    }
  }

}
