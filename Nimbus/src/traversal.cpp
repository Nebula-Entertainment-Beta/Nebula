#include "traversal.h"
#include "component.h"
#include "scriptParams.h"

namespace Nimbus
{

  void syncTraversalFromParams(const Nebula::ScriptComponent &sc,
                               Nebula::ScriptParams &params,
                               TraversalSettings &out)
  {
    out.gravity = params.readScriptParamFloat(sc.paramsJson, "gravity", 20.f);
    out.jumpSpeed = params.readScriptParamFloat(sc.paramsJson, "jumpSpeed", 8.f);
    out.coyoteTime = params.readScriptParamFloat(sc.paramsJson, "coyoteTime", 0.12f);
    out.jumpBufferTime = params.readScriptParamFloat(sc.paramsJson, "jumpBufferTime", 0.12f);
    out.airControlMult = params.readScriptParamFloat(sc.paramsJson, "airControlMult", 0.65f);
    out.fastFallMult = params.readScriptParamFloat(sc.paramsJson, "fastFallMult", 2.5f);
    out.terminalVelocity = params.readScriptParamFloat(sc.paramsJson, "terminalVelocity", 50.f);
    out.killY = params.readScriptParamFloat(sc.paramsJson, "killY", -20.f);
  }

}