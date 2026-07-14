#include "assetBrowserPanel.h"

#include <imgui.h>

namespace Editor
{

  void AssetBrowserPanel::draw(
      AssetCatalog &catalog, EditorState &state, Nebula::Scene &scene, Nebula::AssetManager &assets,
      const std::function<void(std::string_view meshPath, std::string_view materialPath)> &spawnMesh,
      const std::function<void(std::string_view prefabPath)> &spawnPrefab,
      const std::function<void()> &refreshCatalog)
  {
    (void)scene;
    (void)assets;
    ImGui::Begin("Assets");
    if (ImGui::Button("Refresh"))
    {
      refreshCatalog();
    }

    const char *filters[] = {"All", "Meshes", "Materials", "Prefabs"};
    ImGui::Combo("Filter", &state.assetFilter, filters, IM_ARRAYSIZE(filters));

    ImGui::Separator();

    for (const AssetEntry &entry : catalog.entries())
    {
      if (state.assetFilter == 1 && entry.kind != AssetEntryKind::Mesh)
      {
        continue;
      }
      if (state.assetFilter == 2 && entry.kind != AssetEntryKind::Material)
      {
        continue;
      }
      if (state.assetFilter == 3 && entry.kind != AssetEntryKind::Prefab)
      {
        continue;
      }

      ImGui::PushID(entry.logicalPath.c_str());
      if (ImGui::Selectable(entry.label.c_str(), state.selectedAssetPath == entry.logicalPath))
      {
        state.selectedAssetPath = entry.logicalPath;
        state.selectedAssetKind = entry.kind;
      }

      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
      {
        ImGui::SetDragDropPayload("ASSET_PATH", entry.logicalPath.c_str(),
                                  entry.logicalPath.size() + 1);
        ImGui::TextUnformatted(entry.logicalPath.c_str());
        ImGui::EndDragDropSource();
      }

      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
      {
        if (entry.kind == AssetEntryKind::Prefab)
        {
          spawnPrefab(entry.logicalPath);
        }
        else if (entry.kind == AssetEntryKind::Mesh)
        {
          spawnMesh(entry.logicalPath, "materials/solid_cube.mat");
        }
      }
      ImGui::PopID();
    }

    ImGui::End();
  }

} // namespace Editor
