#pragma once

namespace Nebula
{
  struct ScriptComponent;
  class ScriptParams;
}

namespace Nimbus
{

  struct TraversalSettings
  {
    float gravity = 20.f;
    float jumpSpeed = 8.f;
    float coyoteTime = 0.12f;
    float jumpBufferTime = 0.12f;
    float airControlMult = 0.65f;
    float fastFallMult = 2.5f;
    float terminalVelocity = 50.f;
    float killY = -20.f;
  };

  void syncTraversalFromParams(const Nebula::ScriptComponent &sc,
                               Nebula::ScriptParams &params,
                               TraversalSettings &out);

}