#include "sceneViewPanel.h"

#include "collider_debug.h"
#include "editor/editorFlyCamera.h"
#include "editor/editorPicking.h"
#include "input.h"
#include "physics/collision_math.h"
#include "physics/physics_component.h"
#include "transform_gizmo.h"
#include "renderSystem.h"
#include "component.h"

#include <imgui.h>

namespace Editor
{

  namespace
  {
    void initFlyCameraIfNeeded(EditorState &state)
    {
      if (state.flyCameraInitialized)
      {
        return;
      }
      state.flyCamera.position = {4.5f, 3.5f, 6.5f};
      state.flyCamera.yaw = 0.7f;
      state.flyCamera.pitch = -0.3f;
      state.flyCameraInitialized = true;
    }

    ImVec2 mouseInImageRect(const ImVec2 &imageMin, const ImVec2 &imageMax)
    {
      const ImVec2 mouse = ImGui::GetMousePos();
      return {mouse.x - imageMin.x, mouse.y - imageMin.y};
    }

    void updateFlyCameraControls(EditorState &state, Nebula::Input &input, bool hovered, float dt)
    {
      if (!hovered)
      {
        return;
      }

      const ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
      const bool rightMouse = ImGui::IsMouseDown(ImGuiMouseButton_Right);

      if (rightMouse)
      {
        state.flyCamera.addLookDelta(mouseDelta.x * 0.01f, -mouseDelta.y * 0.01f);

        const float speed = (input.isKeyDown(Nebula::Tasto::left_shift) ? 12.0f : 5.0f) * dt;
        float forward = 0.0f;
        float right = 0.0f;
        float up = 0.0f;
        if (input.isKeyDown(Nebula::Tasto::w))
        {
          forward += speed;
        }
        if (input.isKeyDown(Nebula::Tasto::s))
        {
          forward -= speed;
        }
        if (input.isKeyDown(Nebula::Tasto::d))
        {
          right += speed;
        }
        if (input.isKeyDown(Nebula::Tasto::a))
        {
          right -= speed;
        }
        if (input.isKeyDown(Nebula::Tasto::e))
        {
          up += speed;
        }
        if (input.isKeyDown(Nebula::Tasto::q))
        {
          up -= speed;
        }
        state.flyCamera.moveAlongView(forward, right, up);
      }
    }
  }

  void SceneViewPanel::drawSceneViewPanel(
      EditorState &state, SceneViewFrameBuffer &framebuffer, Nebula::Scene &scene,
      Nebula::AssetManager &assets, Nebula::Renderer &renderer, Nebula::Window &window,
      Nebula::Input &input, bool isPlaying, const SpawnMeshFn &spawnMeshAt,
      const SpawnPrefabFn &spawnPrefabAt)
  {
    ImGui::Begin("Scene View");
    initFlyCameraIfNeeded(state);

    if (isPlaying)
    {
      ImGui::TextDisabled("Playing — using game Main Camera");
    }
    else
    {
      ImGui::TextDisabled("RMB+WASD fly | LMB look | click select");
    }

    ImGui::Checkbox("Show Colliders", &state.showColliderGizmos);
    ImGui::SameLine();
    ImGui::Checkbox("Show Transform Gizmo", &state.showTransformGizmo);
    ImGui::SameLine();
    ImGui::Checkbox("Collider Edit", &state.colliderEditMode);

    const char *tools[] = {"Select", "Translate"};
    int toolIndex = state.toolMode == EditorToolMode::Translate ? 1 : 0;
    if (ImGui::Combo("Tool", &toolIndex, tools, IM_ARRAYSIZE(tools)))
    {
      state.toolMode = toolIndex == 1 ? EditorToolMode::Translate : EditorToolMode::Select;
    }

    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0 && size.y > 0)
    {
      const float aspect = size.x / size.y;
      framebuffer.resize(static_cast<int>(size.x), static_cast<int>(size.y));

      const bool useEditorCamera = !isPlaying;
      if (useEditorCamera)
      {
        state.flyCameraViewProjection = state.flyCamera.getViewProjectionMatrix(aspect);
      }

      framebuffer.bind();
      framebuffer.clear();
      Nebula::RenderSystemContext renderCtx{
          scene, assets, renderer, window, 0, 0, static_cast<uint32_t>(size.x),
          static_cast<uint32_t>(size.y), nullptr,
          useEditorCamera ? &state.flyCameraViewProjection : nullptr};
      Nebula::renderScene(renderCtx);
      if (state.showColliderGizmos)
      {
        Nebula::renderColliderGizmos(renderCtx);
      }
      if (!isPlaying && state.showTransformGizmo && scene.isValidEntity(state.selectedEntity))
      {
        Nebula::renderTransformGizmo(renderCtx, state.selectedEntity);
      }
      framebuffer.unbind();

      ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(framebuffer.colorTextureId())),
                   size, ImVec2(0, 1), ImVec2(1, 0));

      const bool hovered = ImGui::IsItemHovered();
      const ImVec2 imageMin = ImGui::GetItemRectMin();
      const ImVec2 localMouse = mouseInImageRect(imageMin, ImGui::GetItemRectMax());

      if (!isPlaying)
      {
        updateFlyCameraControls(state, input, hovered, ImGui::GetIO().DeltaTime);
      }

      static bool dragging = false;
      static bool cameraRotating = false;
      static bool pendingPick = false;
      static ImVec2 clickStart{};
      static GizmoAxis activeAxis = GizmoAxis::None;
      static ColliderHandle activeColliderHandle = ColliderHandle::None;
      static Nebula::Vec3 dragStartPos{};
      static bool dragSymmetric = false;

      const Nebula::Mat4 &viewProjection =
          useEditorCamera ? state.flyCameraViewProjection : Nebula::Mat4{};

      if (!isPlaying && hovered && cameraRotating && ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
        const ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
        state.flyCamera.addLookDelta(mouseDelta.x * 0.01f, -mouseDelta.y * 0.01f);
      }

      if (!isPlaying && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !dragging)
      {
        pendingPick = true;
        cameraRotating = false;
        clickStart = localMouse;

        const ScreenRay ray =
            screenPointToWorldRay(viewProjection, localMouse.x, localMouse.y, size.x, size.y);

        if (state.colliderEditMode && scene.isValidEntity(state.selectedEntity) &&
            scene.hasComponent<Nebula::ColliderComponent>(state.selectedEntity))
        {
          Nebula::CollisionMath collisionMath;
          const Nebula::AABB bounds =
              collisionMath.worldAABBFromEntity(scene, state.selectedEntity);
          activeColliderHandle =
              pickColliderFace(viewProjection, bounds.min, bounds.max, localMouse.x, localMouse.y,
                               size.x, size.y);
          if (activeColliderHandle != ColliderHandle::None)
          {
            dragging = true;
            pendingPick = false;
            dragSymmetric = ImGui::GetIO().KeyShift;
          }
        }

        if (!dragging && state.showTransformGizmo && state.toolMode == EditorToolMode::Translate &&
            scene.isValidEntity(state.selectedEntity) &&
            scene.hasComponent<Nebula::TransformComponent>(state.selectedEntity))
        {
          const Nebula::Vec3 origin =
              scene.getComponent<Nebula::TransformComponent>(state.selectedEntity)
                  .transform.getPosition();
          activeAxis = pickTranslateAxis(viewProjection, origin, localMouse.x, localMouse.y, size.x,
                                         size.y);
          if (activeAxis != GizmoAxis::None)
          {
            dragging = true;
            pendingPick = false;
            dragStartPos = origin;
          }
        }
      }

      if (!isPlaying && hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !dragging)
      {
        const float dx = localMouse.x - clickStart.x;
        const float dy = localMouse.y - clickStart.y;
        if (pendingPick && (dx * dx + dy * dy) > 4.0f)
        {
          pendingPick = false;
          cameraRotating = true;
        }
      }

      if (!isPlaying && dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
        const ScreenRay ray =
            screenPointToWorldRay(viewProjection, localMouse.x, localMouse.y, size.x, size.y);
        if (activeColliderHandle != ColliderHandle::None &&
            scene.hasComponent<Nebula::ColliderComponent>(state.selectedEntity))
        {
          auto &collider = scene.getComponent<Nebula::ColliderComponent>(state.selectedEntity);
          const Nebula::Vec3 entityScale =
              scene.getComponent<Nebula::TransformComponent>(state.selectedEntity).transform.getScale();
          Nebula::CollisionMath collisionMath;
          const Nebula::AABB bounds =
              collisionMath.worldAABBFromEntity(scene, state.selectedEntity);
          const Nebula::Vec3 center = {
              (bounds.min.x + bounds.max.x) * 0.5f,
              (bounds.min.y + bounds.max.y) * 0.5f,
              (bounds.min.z + bounds.max.z) * 0.5f};
          if (dragColliderFace(activeColliderHandle, center, entityScale, dragSymmetric, ray,
                               collider.halfExtents))
          {
            state.sceneDirty = true;
          }
        }
        else if (activeAxis != GizmoAxis::None &&
                 scene.hasComponent<Nebula::TransformComponent>(state.selectedEntity))
        {
          Nebula::Vec3 newPos{};
          if (dragTranslateAxis(activeAxis, ray, dragStartPos, newPos))
          {
            scene.getComponent<Nebula::TransformComponent>(state.selectedEntity)
                .transform.setPosition(newPos);
            state.sceneDirty = true;
          }
        }
      }

      if (!isPlaying && dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        dragging = false;
        activeAxis = GizmoAxis::None;
        activeColliderHandle = ColliderHandle::None;
      }

      if (!isPlaying && hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && pendingPick &&
          !cameraRotating)
      {
        const ScreenRay ray =
            screenPointToWorldRay(viewProjection, localMouse.x, localMouse.y, size.x, size.y);
        const Nebula::Entity picked = pickEntity(scene, assets, ray);
        state.selectedEntity = picked;
      }

      if (!isPlaying && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        pendingPick = false;
        cameraRotating = false;
      }

      if (!isPlaying && ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
        {
          const char *path = static_cast<const char *>(payload->Data);
          const ScreenRay ray =
              screenPointToWorldRay(viewProjection, localMouse.x, localMouse.y, size.x, size.y);
          Nebula::Vec3 hit{};
          if (!rayPlaneYIntersect(ray, 0.0f, hit))
          {
            hit = {0.0f, 0.5f, 0.0f};
          }

          const std::string assetPath(path);
          if (assetPath.find("prefabs/") == 0)
          {
            spawnPrefabAt(assetPath, hit);
          }
          else if (assetPath.find(".mesh") != std::string::npos ||
                   assetPath.find(".obj") != std::string::npos ||
                   assetPath.find(".fbx") != std::string::npos)
          {
            spawnMeshAt(assetPath, "materials/solid_cube.mat", hit);
          }
          state.sceneDirty = true;
        }
        ImGui::EndDragDropTarget();
      }
    }
    ImGui::End();
  }

} // namespace Editor
