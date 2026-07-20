#include "editorApplication.h"

#include "eventBus.h"
#include "eventTypes.h"
#include "sceneSerializer.h"
#include "systemScheduler.h"
#include "prefabSerializer.h"
#include "prefabService.h"
#include "assetProvider.h"
#include "transform_gizmo.h"
#include "renderSystem.h"
#include "component.h"
#include "collider_debug.h"
#include "environment.h"
#include "inputTypes.h"
#include "editor/editorPicking.h"
#include "physics/collision_math.h"

#include <cctype>
#include <filesystem>
#include <cstring>
#include <unordered_map>

namespace Editor
{
  namespace
  {
    std::string toAssetsRelativeScenePath(const std::string &absolutePath)
    {
      const std::filesystem::path path(absolutePath);
      std::filesystem::path relative;
      bool afterAssets = false;
      for (const auto &part : path)
      {
        if (afterAssets)
        {
          relative /= part;
        }
        else if (part == "assets")
        {
          afterAssets = true;
        }
      }
      if (afterAssets && !relative.empty())
      {
        return relative.generic_string();
      }
      return path.filename().generic_string();
    }
  }

  EditorApplication::EditorApplication(const Nebula::ApplicationSpec &spec)
      : Nebula::Application(spec)
  {
  }

  EditorApplication::EditorApplication(const Nebula::ApplicationSpec &spec,
                                       ScriptRegistrar registerScripts,
                                       NewSceneBuilder buildNewScene,
                                       std::vector<ScenePreset> scenePresets)
      : Nebula::Application(spec),
        m_registerScripts(std::move(registerScripts)),
        m_buildNewScene(std::move(buildNewScene)),
        m_scenePresets(std::move(scenePresets))
  {
  }

  EditorApplication::~EditorApplication() = default;

  void EditorLogSink::info(std::string_view msg)
  {
    if (m_log != nullptr)
    {
      m_log->info(msg);
    }
  }

  void EditorLogSink::warn(std::string_view msg)
  {
    if (m_log != nullptr)
    {
      m_log->warn(msg);
    }
  }

  void EditorLogSink::error(std::string_view msg)
  {
    if (m_log != nullptr)
    {
      m_log->error(msg);
    }
  }

  void EditorApplication::onStartup()
  {
    m_scriptLogSink.setLog(&m_editorLog);
    setLogSink(&m_scriptLogSink);

    if (m_registerScripts)
    {
      m_editorLog.info("Registering scripts...");
      m_registerScripts(getScriptRegistry(), getScriptFieldRegistry());
    }

    Nebula::Scene &scene = getScene();
    if (!Nebula::SceneSerializer::load(scene, getAssets(), m_state.scenePath))
    {
      m_editorLog.info("Failed to load scene: " + m_state.scenePath);
      scene.clear();
    }
    m_editorLog.info("Scene loaded: " + m_state.scenePath);

    resolveSceneAssets();
    refreshAssetCatalog();
    Nebula::Application::onStartup();
    m_editorLog.info("Editor started (Qt UI)");
  }

  void EditorApplication::setGizmoMode(int mode)
  {
    m_qtGizmoMode = mode;
    m_state.colliderEditMode = false;
    m_state.toolMode = EditorToolMode::Translate;
    if (mode == 1)
    {
      m_qtGizmo.setMode(GizmoMode::Rotate);
    }
    else if (mode == 2)
    {
      m_qtGizmo.setMode(GizmoMode::Scale);
    }
    else
    {
      m_qtGizmo.setMode(GizmoMode::Translate);
    }
  }

  void EditorApplication::handleQtSceneInteraction()
  {
    if (isPlaying())
    {
      return;
    }

    // Keep the GLFW path as a keyboard fallback when the native child has
    // focus. Pointer interaction is forwarded explicitly from Qt.
    if (getInput().wasKeyPressed(Nebula::Tasto::w) && !m_qtRightDragging)
    {
      setGizmoMode(0);
    }
    if (getInput().wasKeyPressed(Nebula::Tasto::e) && !m_qtRightDragging)
    {
      setGizmoMode(1);
    }
    if (getInput().wasKeyPressed(Nebula::Tasto::r) && !m_qtRightDragging)
    {
      setGizmoMode(2);
    }

    const float move = 8.f * m_lastFrameDt;
    if (m_qtRightDragging)
    {
      m_state.flyCamera.moveAlongView(
          (m_qtMoveForward ? move : 0.f) - (m_qtMoveBackward ? move : 0.f),
          (m_qtMoveRight ? move : 0.f) - (m_qtMoveLeft ? move : 0.f),
          (m_qtMoveUp ? move : 0.f) - (m_qtMoveDown ? move : 0.f));
    }
  }

  void EditorApplication::scenePointerPress(float x, float y, bool left, bool right,
                                            bool shift, int width, int height)
  {
    if (isPlaying() || width <= 0 || height <= 0)
    {
      return;
    }
    m_qtPointerX = x;
    m_qtPointerY = y;
    if (right)
    {
      m_qtRightDragging = true;
      // Fresh fly session: movement only comes from keys pressed while RMB is
      // held, so stale flags can never push the camera on a plain right-click.
      m_qtMoveForward = m_qtMoveBackward = m_qtMoveLeft = m_qtMoveRight = false;
      m_qtMoveUp = m_qtMoveDown = false;
    }
    if (!left)
    {
      return;
    }

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    m_state.flyCameraViewProjection = m_state.flyCamera.getViewProjectionMatrix(aspect);
    const ScreenRay ray =
        screenPointToWorldRay(m_state.flyCameraViewProjection, x, y,
                              static_cast<float>(width), static_cast<float>(height));
    m_qtPendingPick = true;
    m_qtClickX = x;
    m_qtClickY = y;

    Nebula::Scene &scene = getScene();
    if (m_state.colliderEditMode && scene.isValidEntity(m_state.selectedEntity) &&
        scene.hasComponent<Nebula::ColliderComponent>(m_state.selectedEntity))
    {
      Nebula::CollisionMath collisionMath;
      const Nebula::AABB bounds =
          collisionMath.worldAABBFromEntity(scene, m_state.selectedEntity);
      m_qtColliderHandle =
          pickColliderFace(m_state.flyCameraViewProjection, bounds.min, bounds.max, x, y,
                           static_cast<float>(width), static_cast<float>(height));
      if (m_qtColliderHandle != ColliderHandle::None)
      {
        m_qtColliderSymmetric = shift;
        m_qtPendingPick = false;
        return;
      }
    }

    if (m_state.toolMode == EditorToolMode::Translate &&
        m_state.showTransformGizmo && scene.isValidEntity(m_state.selectedEntity) &&
        scene.hasComponent<Nebula::TransformComponent>(m_state.selectedEntity))
    {
      const auto &tf =
          scene.getComponent<Nebula::TransformComponent>(m_state.selectedEntity).transform;
      const GizmoAxis axis =
          m_qtGizmo.hitTest(m_qtGizmo.mode(), m_state.flyCameraViewProjection,
                            tf.getPosition(), x, y, static_cast<float>(width),
                            static_cast<float>(height));
      if (axis != GizmoAxis::None)
      {
        m_qtGizmo.beginDrag(axis, tf.getPosition(), tf.getYaw(), tf.getScale(), ray);
        m_qtPendingPick = false;
      }
    }
  }

  void EditorApplication::scenePointerMove(float x, float y, bool left, bool right,
                                           int width, int height)
  {
    if (isPlaying() || width <= 0 || height <= 0)
    {
      return;
    }
    const float dx = x - m_qtPointerX;
    const float dy = y - m_qtPointerY;
    m_qtPointerX = x;
    m_qtPointerY = y;

    if (right || m_qtRightDragging)
    {
      m_qtRightDragging = true;
      m_state.flyCamera.addLookDelta(dx * 0.005f, -dy * 0.005f);
    }
    if (left && m_qtPendingPick && !m_qtGizmo.isDragging() &&
        m_qtColliderHandle == ColliderHandle::None)
    {
      const float totalX = x - m_qtClickX;
      const float totalY = y - m_qtClickY;
      if (totalX * totalX + totalY * totalY > 16.f)
      {
        m_qtPendingPick = false;
        m_qtLeftLooking = true;
      }
    }
    if (left && m_qtLeftLooking)
    {
      m_state.flyCamera.addLookDelta(dx * 0.005f, -dy * 0.005f);
      return;
    }

    if (!left)
    {
      return;
    }
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    m_state.flyCameraViewProjection = m_state.flyCamera.getViewProjectionMatrix(aspect);
    const ScreenRay ray =
        screenPointToWorldRay(m_state.flyCameraViewProjection, x, y,
                              static_cast<float>(width), static_cast<float>(height));
    Nebula::Scene &scene = getScene();
    if (m_qtColliderHandle != ColliderHandle::None &&
        scene.isValidEntity(m_state.selectedEntity) &&
        scene.hasComponent<Nebula::TransformComponent>(m_state.selectedEntity) &&
        scene.hasComponent<Nebula::ColliderComponent>(m_state.selectedEntity))
    {
      auto &collider =
          scene.getComponent<Nebula::ColliderComponent>(m_state.selectedEntity);
      const Nebula::Vec3 entityScale =
          scene.getComponent<Nebula::TransformComponent>(m_state.selectedEntity)
              .transform.getScale();
      Nebula::CollisionMath collisionMath;
      const Nebula::AABB bounds =
          collisionMath.worldAABBFromEntity(scene, m_state.selectedEntity);
      const Nebula::Vec3 center = {
          (bounds.min.x + bounds.max.x) * 0.5f,
          (bounds.min.y + bounds.max.y) * 0.5f,
          (bounds.min.z + bounds.max.z) * 0.5f};
      if (dragColliderFace(m_qtColliderHandle, center, entityScale,
                           m_qtColliderSymmetric, ray, collider.halfExtents))
      {
        m_state.sceneDirty = true;
      }
      return;
    }

    if (!m_qtGizmo.isDragging())
    {
      return;
    }
    if (!scene.isValidEntity(m_state.selectedEntity) ||
        !scene.hasComponent<Nebula::TransformComponent>(m_state.selectedEntity))
    {
      return;
    }
    Nebula::Vec3 pos{};
    float yaw = 0.f;
    Nebula::Vec3 scale{};
    if (m_qtGizmo.updateDrag(ray, pos, yaw, scale))
    {
      auto &tf =
          scene.getComponent<Nebula::TransformComponent>(m_state.selectedEntity).transform;
      tf.setPosition(pos);
      tf.setYaw(yaw);
      tf.setScale(scale);
      m_state.sceneDirty = true;
    }
  }

  void EditorApplication::scenePointerRelease(float x, float y, bool left, bool right,
                                              int width, int height)
  {
    if (right)
    {
      m_qtRightDragging = false;
      m_qtMoveForward = m_qtMoveBackward = m_qtMoveLeft = m_qtMoveRight = false;
      m_qtMoveUp = m_qtMoveDown = false;
    }
    if (isPlaying() || !left || width <= 0 || height <= 0)
    {
      return;
    }
    m_qtLeftLooking = false;

    const bool wasDragging =
        m_qtGizmo.isDragging() || m_qtColliderHandle != ColliderHandle::None;
    if (wasDragging)
    {
      m_qtGizmo.endDrag();
      m_qtColliderHandle = ColliderHandle::None;
    }
    else if (m_qtPendingPick)
    {
      const float dx = x - m_qtClickX;
      const float dy = y - m_qtClickY;
      if (dx * dx + dy * dy <= 25.f)
      {
        const float aspect = static_cast<float>(width) / static_cast<float>(height);
        m_state.flyCameraViewProjection = m_state.flyCamera.getViewProjectionMatrix(aspect);
        const ScreenRay ray =
            screenPointToWorldRay(m_state.flyCameraViewProjection, x, y,
                                  static_cast<float>(width), static_cast<float>(height));
        m_state.selectedEntity = pickEntity(getScene(), getAssetManager(), ray);
      }
    }
    m_qtPendingPick = false;
  }

  void EditorApplication::sceneWheel(float delta)
  {
    if (!isPlaying())
    {
      m_state.flyCamera.moveAlongView(delta * 0.0025f, 0.f, 0.f);
    }
  }

  void EditorApplication::sceneKeyChanged(int key, bool down)
  {
    // Qt letter-key values match uppercase ASCII.
    switch (key)
    {
    case 'W':
      m_qtMoveForward = down;
      if (down && !m_qtRightDragging)
      {
        setGizmoMode(0);
      }
      break;
    case 'S':
      m_qtMoveBackward = down;
      break;
    case 'A':
      m_qtMoveLeft = down;
      break;
    case 'D':
      m_qtMoveRight = down;
      break;
    case 'Q':
      m_qtMoveDown = down;
      break;
    case 'Z':
      m_qtMoveUp = down;
      break;
    case 'E':
      if (down && !m_qtRightDragging)
      {
        setGizmoMode(1);
      }
      break;
    case 'R':
      if (down && !m_qtRightDragging)
      {
        setGizmoMode(2);
      }
      break;
    default:
      break;
    }
  }

  void EditorApplication::sceneDropAsset(std::string_view path, AssetEntryKind kind,
                                         float x, float y, int width, int height)
  {
    if (isPlaying() || width <= 0 || height <= 0)
    {
      return;
    }
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    m_state.flyCameraViewProjection = m_state.flyCamera.getViewProjectionMatrix(aspect);
    const ScreenRay ray =
        screenPointToWorldRay(m_state.flyCameraViewProjection, x, y,
                              static_cast<float>(width), static_cast<float>(height));
    Nebula::Vec3 position{0.f, 0.5f, 0.f};
    Nebula::Vec3 hit{};
    if (rayPlaneYIntersect(ray, 0.f, hit))
    {
      position = hit;
      position.y = 0.5f;
    }
    if (kind == AssetEntryKind::Prefab)
    {
      spawnPrefabAt(path, position);
    }
    else if (kind == AssetEntryKind::Mesh)
    {
      spawnMeshAt(path, "materials/solid_cube.mat", position);
    }
  }

  void EditorApplication::onRender()
  {
    Nebula::Application::onRender();
    if (!isPlaying())
    {
      int fbw = 0;
      int fbh = 0;
      getWindow().getFramebufferSize(fbw, fbh);
      const float aspect =
          (fbh > 0) ? (static_cast<float>(fbw) / static_cast<float>(fbh)) : (16.f / 9.f);
      if (!m_state.flyCameraInitialized)
      {
        m_state.flyCamera.position = {4.f, 8.f, 22.f};
        m_state.flyCamera.yaw = 3.14159f;
        m_state.flyCamera.pitch = -0.30f;
        m_state.flyCameraInitialized = true;
      }
      m_state.flyCameraViewProjection = m_state.flyCamera.getViewProjectionMatrix(aspect);
      Nebula::RenderSystemContext ctx{getScene(), getAssetManager(), getRenderer(), getWindow()};
      ctx.overrideViewProjection = &m_state.flyCameraViewProjection;
      getRenderer().clear(Nebula::environmentClearColor(Nebula::findEnvironmentOrDefault(getScene())));
      Nebula::renderScene(ctx);
      if (m_state.showColliderGizmos)
      {
        Nebula::renderColliderGizmos(ctx);
      }
      if (m_state.showTransformGizmo && m_state.selectedEntity.id != 0)
      {
        Nebula::renderTransformGizmo(ctx, m_state.selectedEntity, m_qtGizmoMode);
      }
    }
  }

  void EditorApplication::enterPlayMode()
  {
    m_sceneSerializer.saveToString(m_playmode.snapshot(), getScene(), getAssetManager());
    setPlaying(true);
    rebuildScripts();
    activateScripts();
    m_editorLog.info("Play");
  }

  void EditorApplication::exitPlayMode()
  {
    setPlaying(false);
    Nebula::ScriptContext ctx = makeScriptContext();
    getScriptSystem().shutdownAll(ctx);
    m_sceneSerializer.loadFromString(getScene(), m_playmode.snapshot(), getAssets());
    resolveSceneAssets();
    rebuildScripts(); // bind only; lifecycle waits for next Play
    m_editorLog.info("Stop");
  }

  bool EditorApplication::loadSceneFromAbsolutePath(const std::string &absolutePath)
  {
    Nebula::FileAssetProvider *files = fileAssets();
    if (files == nullptr)
    {
      return false;
    }

    std::string logicalPath = absolutePath;
    for (const auto &root : files->searchRoots())
    {
      std::error_code ec;
      const auto rel = std::filesystem::relative(absolutePath, root, ec);
      if (!ec)
      {
        logicalPath = rel.generic_string();
        break;
      }
    }

    if (!loadScene(logicalPath))
    {
      m_editorLog.info("Failed to load scene: " + logicalPath);
      return false;
    }
    resolveSceneAssets();
    Nebula::Application::onStartup();
    m_editorLog.info("Scene loaded: " + logicalPath);
    return true;
  }

  Nebula::FileAssetProvider *EditorApplication::fileAssets()
  {
    return dynamic_cast<Nebula::FileAssetProvider *>(&getAssets());
  }

  void EditorApplication::refreshAssetCatalog()
  {
    if (Nebula::FileAssetProvider *files = fileAssets())
    {
      m_assetCatalog.refresh(*files);
    }
  }

  void EditorApplication::spawnMeshAt(std::string_view meshPath, std::string_view materialPath,
                                      const Nebula::Vec3 &position)
  {
    Nebula::Entity entity =
        m_template.createStaticMesh(getScene(), getAssetManager(), position, meshPath, materialPath);
    m_state.selectedEntity = entity;
    m_state.sceneDirty = true;
    resolveSceneAssets();
  }

  void EditorApplication::spawnPrefabAt(std::string_view prefabPath, const Nebula::Vec3 &position)
  {
    Nebula::Entity entity = instantiatePrefab(prefabPath);
    if (entity.id != 0 && getScene().hasComponent<Nebula::TransformComponent>(entity))
    {
      getScene().getComponent<Nebula::TransformComponent>(entity).transform.setPosition(position);
      m_state.sceneDirty = true;
    }
  }

  void EditorApplication::newScene()
  {
    if (m_buildNewScene)
    {
      newScene(m_buildNewScene);
      return;
    }

    getScene().clear();
    m_state.selectedEntity = {};
    m_state.sceneDirty = true;
    resolveSceneAssets();
    Nebula::Application::onStartup();
    m_editorLog.info("New scene created");
  }

  void EditorApplication::newScene(NewSceneBuilder builder)
  {
    getScene().clear();
    m_state.selectedEntity = {};

    if (builder)
    {
      builder(getScene());
    }

    m_state.sceneDirty = true;
    resolveSceneAssets();
    Nebula::Application::onStartup();

    m_editorLog.info("New scene created");
  }

  void EditorApplication::newSceneFromPath(std::string_view path)
  {
    getScene().clear();
    m_state.selectedEntity = {};

    if (!Nebula::SceneSerializer::load(getScene(), getAssets(), path))
    {
      m_editorLog.info("Failed to load scene: " + std::string(path));
      return;
    }

    m_state.scenePath = std::string(path);
    m_state.sceneDirty = false;
    resolveSceneAssets();
    Nebula::Application::onStartup();
    m_editorLog.info("Scene loaded: " + std::string(path));
  }

  void EditorApplication::createEmptyEntity()
  {
    createEntityFromTemplate("empty");
  }

  bool EditorApplication::saveSelectedAsPrefab(std::string_view path)
  {
    if (!getScene().isValidEntity(m_state.selectedEntity))
    {
      return false;
    }
    if (!Nebula::PrefabSerializer::save(getScene(), m_state.selectedEntity,
                                        getAssetManager(), getAssets(), path))
    {
      return false;
    }
    m_editorLog.info("Saved prefab: " + std::string(path));
    return true;
  }

  void EditorApplication::saveSelectedEntityAsPrefab()
  {
    Nebula::Scene &scene = getScene();
    if (!scene.isValidEntity(m_state.selectedEntity))
    {
      return;
    }

    std::string path = "prefabs/";
    if (scene.hasComponent<Nebula::TagComponent>(m_state.selectedEntity))
    {
      for (char c : scene.getComponent<Nebula::TagComponent>(m_state.selectedEntity).tag)
      {
        path.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
    }
    else
    {
      path += "entity_" + std::to_string(m_state.selectedEntity.id);
    }
    path += ".prefab";

    if (saveSelectedAsPrefab(path))
    {
      m_state.sceneDirty = true;
    }
  }

  Nebula::Entity EditorApplication::instantiatePrefab(std::string_view path)
  {
    Nebula::Entity e = Nebula::PrefabService::instantiate(
        getScene(), getAssetManager(), getAssets(), getRenderer().resources(), path);
    if (e.id != 0)
    {
      m_state.selectedEntity = e;
      m_state.sceneDirty = true;
      resolveSceneAssets();
    }
    return e;
  }

  void EditorApplication::revertSelectedPrefabInstance()
  {
    if (!Nebula::PrefabService::revertInstance(getScene(), m_state.selectedEntity, getAssetManager(),
                                               getAssets(), getRenderer().resources()))
    {
      return;
    }
    m_state.sceneDirty = true;
    resolveSceneAssets();
    m_editorLog.info("Reverted prefab instance");
  }

  void EditorApplication::createVariantFromSelectedInstance()
  {
    Nebula::Scene &scene = getScene();
    if (!scene.isValidEntity(m_state.selectedEntity) ||
        !scene.hasComponent<Nebula::PrefabInstanceComponent>(m_state.selectedEntity))
    {
      return;
    }

    std::string path = "prefabs/";
    if (scene.hasComponent<Nebula::TagComponent>(m_state.selectedEntity))
    {
      for (char c : scene.getComponent<Nebula::TagComponent>(m_state.selectedEntity).tag)
      {
        path.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
    }
    else
    {
      path += "entity_" + std::to_string(m_state.selectedEntity.id);
    }
    path += "_variant.prefab";

    if (Nebula::PrefabService::saveVariantFromInstance(scene, m_state.selectedEntity, getAssetManager(),
                                                        getAssets(), path))
    {
      m_editorLog.info("Saved prefab variant: " + path);
    }
  }

  bool EditorApplication::resetSelectedOverridePublic(const char *component,
                                                      const char *field)
  {
    if (!getScene().isValidEntity(m_state.selectedEntity))
    {
      return false;
    }
    const bool reset = Nebula::PrefabService::resetInstanceOverrideField(
        getScene(), m_state.selectedEntity, getAssetManager(), getAssets(), component,
        field);
    if (reset)
    {
      m_state.sceneDirty = true;
      resolveSceneAssets();
    }
    return reset;
  }

  void EditorApplication::deleteSelectedEntity()
  {
    if (m_state.selectedEntity == Nebula::Entity())
    {
      return;
    }
    Nebula::Scene &scene = getScene();
    if (!scene.isValidEntity(m_state.selectedEntity))
    {
      m_state.selectedEntity = {};
      return;
    }
    scene.destroyEntity(m_state.selectedEntity);
    m_state.selectedEntity = {};
    m_state.sceneDirty = true;
  }

  bool EditorApplication::saveScene()
  {
    if (!Nebula::SceneSerializer::save(getScene(), getAssetManager(), getAssets(), m_state.scenePath))
      return false;
    m_state.sceneDirty = false;
    getEventBus().push(Nebula::SceneSavedEvent{m_state.scenePath});
    m_editorLog.info("Saved " + m_state.scenePath);
    return true;
  }

  bool EditorApplication::saveSceneAsAbsolutePublic(const std::string &path)
  {
    m_state.scenePath = toAssetsRelativeScenePath(path);
    return saveScene();
  }

  bool EditorApplication::loadSceneAbsolutePublic(const std::string &path)
  {
    return loadSceneFromAbsolutePath(path);
  }

  bool EditorApplication::loadScene(std::string_view path)
  {
    getScene().clear();
    if (!Nebula::SceneSerializer::load(getScene(), getAssets(), path))
      return false;
    getAssetManager().resolveScene(getScene(), getRenderer().resources());
    m_state.scenePath = std::string(path);
    m_state.selectedEntity = {};
    m_state.sceneDirty = false;
    return true;
  }

  void EditorApplication::registerGameSystems()
  {
    getScheduler().add(Nebula::SystemPhase::PreUpdate, [this](float)
                       {
                         for (const Nebula::GameEvent &event : getEventBus().events())
                         {
                           if (std::holds_alternative<Nebula::SaveSceneRequestedEvent>(event))
                           {
                             saveScene();
                             break;
                           }
                         } });
    getScheduler().add(Nebula::SystemPhase::Update, [this](float dt)
                       {
                         m_lastFrameDt = dt > 0.f ? dt : 0.016f;
                         handleQtSceneInteraction();
                       });
  }

  void EditorApplication::createEntityFromTemplate(const char *id)
  {
    Nebula::Scene &scene = getScene();
    Nebula::Entity e{};

    static const std::unordered_map<std::string, const char *> kPrefabForTemplate = {
        {"enemy", "prefabs/enemy.prefab"},
        {"platform", "prefabs/platform.prefab"},
        {"bouncePad", "prefabs/bounce_pad.prefab"},
        {"windVolume", "prefabs/wind_volume.prefab"},
    };
    if (auto it = kPrefabForTemplate.find(id); it != kPrefabForTemplate.end())
    {
      instantiatePrefab(it->second);
      return;
    }
    if (strcmp(id, "cube") == 0)
      e = m_template.createMeshCube(scene);

    else
      e = m_template.createEmpty(scene); // must add Transform inside
    m_state.selectedEntity = e;
    m_state.sceneDirty = true;
    resolveSceneAssets();
  }

} // namespace Editor
