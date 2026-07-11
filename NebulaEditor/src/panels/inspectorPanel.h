#pragma once
#include "editorState.h"
#include "Nebula.h"
#include "scriptParams.h"
#include "scriptFields.h"
#include "assetProvider.h"
#include "renderResources.h"

namespace Editor
{
  class sceneViewFrameBuffer;

  class InspectorPanel
  {

  public:
    void drawInspectorPanel(EditorState &state, Nebula::Scene &scene,
                            Nebula::Entity entity, Nebula::ScriptFieldRegistry &scriptFieldRegistry,
                            Nebula::ScriptRegistry &scriptRegistry, Nebula::AssetManager &assets,
                            Nebula::IAssetProvider &fileProvider,
                            Nebula::IRenderResourceFactory &renderResources,
                            std::function<void()> onSceneEdited,
                            std::function<void()> onRevertPrefab,
                            std::function<void()> onCreateVariant);
    void drawScriptFields(Nebula::Entity entity, Nebula::ScriptFieldRegistry &fieldRegistry, Nebula::Scene &scene,
                          EditorState &state, bool isPrefabInstance, Nebula::AssetManager &assets,
                          Nebula::IAssetProvider &fileProvider);

  private:
    bool drawComponentHeaderWithRemove(const char *title, const char *buttonId);
    void drawPrefabInstanceHeader(Nebula::Scene &scene, Nebula::Entity entity, EditorState &state,
                                  std::function<void()> onRevertPrefab, std::function<void()> onCreateVariant);
    void drawTransformFields(Nebula::Scene &scene, Nebula::Entity entity, EditorState &state, bool isPrefabInstance,
                             Nebula::AssetManager &assets, Nebula::IAssetProvider &fileProvider);
    void drawMeshRendererFields(Nebula::MeshRendererComponent &meshRenderer, EditorState &state,
                                const std::function<void()> &onSceneEdited);
    void drawScriptSelector(Nebula::ScriptComponent &script, Nebula::ScriptFieldRegistry &fieldRegistry,
                            Nebula::ScriptRegistry &scriptRegistry, EditorState &state);
    void drawCameraFields(Nebula::CameraComponent &camera, EditorState &state);
    void drawColliderFields(Nebula::ColliderComponent &collider, Nebula::Scene &scene, Nebula::Entity entity,
                            Nebula::AssetManager &assets, EditorState &state,
                            const std::function<void()> &onSceneEdited);
    void drawRigidBodyFields(Nebula::RigidBodyComponent &body, EditorState &state);
    void drawEntityVectorField(Nebula::Entity owner, const Nebula::ScriptFieldDescriptor &field,
                               Nebula::ScriptComponent &script, Nebula::Scene &scene, EditorState &state,
                               bool isPrefabInstance);
    bool isOverrideActive(Nebula::Scene &scene, Nebula::Entity entity, const char *componentKey,
                          const char *fieldKey) const;
    void drawOverrideResetButton(Nebula::Scene &scene, Nebula::Entity entity, EditorState &state,
                                 Nebula::AssetManager &assets, Nebula::IAssetProvider &fileProvider,
                                 const char *componentKey, const char *fieldKey);
    std::string entityLabel(Nebula::Scene &scene, Nebula::Entity entity) const;

    Nebula::ScriptParams m_scriptParams;
  };

}
