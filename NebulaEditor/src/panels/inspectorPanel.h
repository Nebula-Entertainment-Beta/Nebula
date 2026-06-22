#pragma once
#include "editorState.h"
#include "Nebula.h"
#include "scriptParams.h"
#include "scriptFields.h"

namespace Editor
{
  class sceneViewFrameBuffer;

  class InspectorPanel
  {

  public:
    void drawInspectorPanel(EditorState &state, Nebula::Scene &scene,
                            Nebula::Entity entity, Nebula::ScriptFieldRegistry &scriptFieldRegistry,
                            Nebula::ScriptRegistry &scriptRegistry, std::function<void()> onSceneEdited);
    // script fields use existingregistry+ params json
    void drawScriptFields(Nebula::Entity entity, Nebula::ScriptFieldRegistry &fieldRegistry, Nebula::Scene &scene, EditorState &state);

  private:
    bool drawComponentHeaderWithRemove(const char *title, const char *buttonId);
    void drawMeshRendererFields(Nebula::MeshRendererComponent &meshRenderer, EditorState &state,
                                const std::function<void()> &onSceneEdited);
    void drawScriptSelector(Nebula::ScriptComponent &script, Nebula::ScriptFieldRegistry &fieldRegistry,
                            Nebula::ScriptRegistry &scriptRegistry, EditorState &state);
    void drawCameraFields(Nebula::CameraComponent &camera, EditorState &state);
    void drawColliderFields(Nebula::ColliderComponent &collider, EditorState &state);
    void drawRigidBodyFields(Nebula::RigidBodyComponent &body, EditorState &state);

    Nebula::ScriptParams m_scriptParams;
  };

}