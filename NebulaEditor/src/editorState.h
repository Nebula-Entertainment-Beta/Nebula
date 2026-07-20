#pragma once

#include <string>
#include <Nebula.h>
#include "editor/assetCatalog.h"
#include "editor/editorFlyCamera.h"

namespace Editor
{

  enum class EditorToolMode
  {
    Select,
    Translate,
    ColliderEdit
  };

  struct EditorState
  {
    Nebula::Entity selectedEntity{};
    std::string scenePath = "scenes/vertical_slice.json";
    bool sceneDirty = false;
    bool showColliderGizmos = true;
    bool showTransformGizmo = true;
    bool colliderEditMode = false;
    EditorToolMode toolMode = EditorToolMode::Select;

    EditorFlyCamera flyCamera{};
    bool flyCameraInitialized = false;
    Nebula::Mat4 flyCameraViewProjection{};

    int assetFilter = 0;
    std::string selectedAssetPath;
    AssetEntryKind selectedAssetKind = AssetEntryKind::Mesh;
  };

}
