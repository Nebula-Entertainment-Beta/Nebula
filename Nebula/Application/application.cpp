/**
 * @file application.cpp
 * @brief `Application` constructor, `run()` loop (`Renderer::init` … `Shutdown`), and default `onRender` (viewport + clear).
 */
#include "application.h"
#include "renderer.h"
#include "renderSystem.h"
#include "physics/iphysics_world.h"
#include "physics/physics_system.h"
#include "physics_query_adapter.h"
#include "math_types.h"
#include "audioService.h"
#include "environment.h"

namespace Nebula
{

  Application::Application(const ApplicationSpec &spec)
      : m_window(WindowSpec{spec.title, spec.width, spec.height, spec.rendererAPI}),
        m_assets(), m_assetManager(m_assets),
        m_world(m_scene, m_assets, m_input, m_actionMapping, m_scriptRegistry, m_frameInput)
  {
    m_width = spec.width;
    m_height = spec.height;
    m_title = spec.title;
    m_rendererAPI = spec.rendererAPI;

    m_input.attach(m_window);

    // create default scene data
    m_scene = Scene();
    m_physicsWorld = createPhysXWorld();
    if (!m_physicsWorld)
    {
      m_physicsWorld = createNullPhysicsWorld();
    }
    m_physicsQuery = std::make_unique<PhysicsQueryAdapter>(*m_physicsWorld);
    m_audio = createAudioService();
  }

  bool Application::startFrameLoop()
  {
    if (m_frameLoopActive || m_hasRun)
    {
      return m_frameLoopActive;
    }
    if (!m_window.isValid())
    {
      return false;
    }
    auto &ctx = m_window.getGraphicsContext();
    ctx.makeCurrent();
    m_renderer.init(ctx, m_rendererAPI);
    m_assetManager.loadBuiltins(m_renderer.resources());
    m_assetManager.resolveScene(m_scene, m_renderer.resources());
    m_rendererInitialized = true;
    m_hasRun = true;
    m_frameLoopActive = true;
    m_lastFrameTime = m_clock.nowSeconds();
    registerEngineSystems();
    registerGameSystems();
    onStartup();
    return true;
  }

  bool Application::pumpFrame()
  {
    if (!m_frameLoopActive || !m_window.isValid() || m_window.shouldWindowClose())
    {
      return false;
    }
    auto &ctx = m_window.getGraphicsContext();
    const float newtime = m_clock.nowSeconds();
    const float dt = newtime - m_lastFrameTime;
    m_lastFrameTime = newtime;
    // Poll after any events that Qt's message loop may already have delivered into
    // GLFW callbacks between frames. Clear edge/delta state only after consumers run,
    // otherwise Qt+embedded HWND input is wiped before Update sees it.
    m_window.pollEvents();
    m_actionMapping.updateMappings(m_input);
    m_scheduler.run(SystemPhase::PreUpdate, dt);
    m_scheduler.run(SystemPhase::Update, dt);
    m_scheduler.runFixed(SystemPhase::FixedUpdate, 1.f / 60.f, dt);
    m_scheduler.run(SystemPhase::PostUpdate, dt);
    m_scheduler.run(SystemPhase::Render, dt);
    if (m_audio)
    {
      m_audio->update();
    }
    ctx.swap();
    m_input.beginFrame();
    return true;
  }

  void Application::stopFrameLoop()
  {
    if (!m_frameLoopActive)
    {
      return;
    }
    ScriptContext shutdownCtx = makeScriptContext();
    m_scriptSystem.shutdownAll(shutdownCtx);
    m_renderer.Shutdown();
    m_rendererInitialized = false;
    m_frameLoopActive = false;
  }

  void Application::run()
  {
    if (!startFrameLoop())
    {
      return;
    }
    while (pumpFrame())
    {
    }
    stopFrameLoop();
  }

  Application::~Application()
  {

    if (m_rendererInitialized)
    {
      ScriptContext contx = makeScriptContext();

      m_scriptSystem.shutdownAll(contx);
      m_renderer.Shutdown();
    }
    m_input.detach();
    m_rendererInitialized = false;
  }

  void Application::registerGameSystems()
  {
  }

  void Application::rebuildScripts()
  {
    ScriptContext ctx = makeScriptContext();
    m_scriptSystem.rebuildFromScene(m_scene, m_scriptRegistry, ctx);
    if (m_rendererInitialized)
    {
      m_assetManager.resolveSceneAssets(m_scene, m_renderer.resources());
    }
  }

  void Application::activateScripts()
  {
    ScriptContext ctx = makeScriptContext();
    m_scriptSystem.initializeAll(ctx);
    m_scheduler.resetFixedAccumulator();
    m_world.prevJumpDown() = false;
  }

  void Application::bindNewScripts()
  {
    ScriptContext ctx = makeScriptContext();
    m_scriptSystem.bindNewFromScene(m_scene, m_scriptRegistry, ctx, m_isPlaying);
    if (m_rendererInitialized)
    {
      m_assetManager.resolveSceneAssets(m_scene, m_renderer.resources());
    }
  }

  void Application::queueScriptRebuild()
  {
    m_pendingScriptRebuild = true;
  }

  void Application::onRequestScriptRebuild(void *userData)
  {
    static_cast<Application *>(userData)->queueScriptRebuild();
  }

  void Application::onStartup()
  {
    rebuildScripts();
    if (m_isPlaying)
    {
      activateScripts();
    }
  }

  void Application::onUpdate(float dt)
  {
  }
  void Application::onRender()
  {
    int fbw = 0;
    int fbh = 0;
    m_window.getFramebufferSize(fbw, fbh);
    if (fbw > 0 && fbh > 0)
    {
      if (fbw != m_lastFbWidth || fbh != m_lastFbHeight)
      {
        m_lastFbWidth = fbw;
        m_lastFbHeight = fbh;
        m_eventBus.push(WindowResizedEvent{fbw, fbh});
      }
      m_renderer.setViewport(0, 0, static_cast<uint32_t>(fbw), static_cast<uint32_t>(fbh));
    }
    else
    {
      m_renderer.setViewport(0, 0, m_width, m_height);
    }
    const EnvironmentComponent env = findEnvironmentOrDefault(m_scene);
    m_renderer.clear(environmentClearColor(env));
  }

  ScriptContext Application::makeScriptContext()
  {
    ScriptContext ctx{m_sceneAccess, &m_inputQuery, m_logSink};
    ctx.physics = m_physicsQuery.get();
    ctx.physicsScene = &m_scene;
    ctx.assetManager = &m_assetManager;
    ctx.assets = &m_assets;
    ctx.audio = m_audio.get();
    if (m_rendererInitialized)
    {
      ctx.renderResources = &m_renderer.resources();
    }
    ctx.scriptRebuildUserData = this;
    ctx.requestScriptRebuildFn = &Application::onRequestScriptRebuild;
    return ctx;
  }

  void buildFrameInput(World &world, EventBus &bus)
  {
    FrameInput &f = world.frameInput();
    f.clear();

    Input &input = world.input();
    ActionMapping &map = world.actions();

    if (map.wasActionPressed(Action::LightAttack, input))
    {
      f.lightAttackPressed = true;
    }

    if (map.wasActionPressed(Action::HeavyAttack, input))
    {
      f.heavyAttackPressed = true;
    }

    bool &prevJumpDown = world.prevJumpDown();
    const bool jumpDown = map.isActionDown(Action::Jump, input);
    if (map.wasActionPressed(Action::Jump, input) || (jumpDown && !prevJumpDown))
    {
      f.jumpPressed = true;
    }
    f.jumpHeld = jumpDown;
    prevJumpDown = jumpDown;

    if (map.isActionDown(Action::FastFall, input))
    {
      f.fastFall = true;
    }

    if (map.wasActionPressed(Action::Interact, input))
    {

      bus.push(InteractPressedEvent{});
    }

    if (map.wasActionPressed(Action::SaveScene, input))
      bus.push(SaveSceneRequestedEvent{});

    map.getAxisValue(Axis::LookX, input, f.lookX, f.lookY);
    map.getAxisValue(Axis::LookY, input, f.lookX, f.lookY);
    map.getAxisValue(Axis::MoveX, input, f.moveX, f.moveY);
    map.getAxisValue(Axis::MoveY, input, f.moveX, f.moveY);
    map.getAxisValue(Axis::Scroll, input, f.zoomX, f.zoomY);
  }

  void Application::registerEngineSystems()
  {
    m_scheduler.add(SystemPhase::PreUpdate, [this](float)
                    {
                      m_eventBus.clear();
                      buildFrameInput(m_world, m_eventBus); });

    m_scheduler.add(SystemPhase::Update, [this](float dt)
                    {
                      if(m_isPlaying)
                      {
                        ScriptContext ctx = makeScriptContext();
                        m_scriptSystem.updateAll(ctx, dt);
                      } });

    m_scheduler.add(SystemPhase::PostUpdate, [this](float)
                    {
                      if (m_pendingScriptRebuild)
                      {
                        m_pendingScriptRebuild = false;
                        bindNewScripts();
                      } });

    m_scheduler.add(SystemPhase::FixedUpdate, [this](float fdt)
                    { 
                      if(m_isPlaying)
                      {
                        runPhysicsFixedUpdate(m_scene, *m_physicsWorld, fdt);
                      } });

    m_scheduler.add(SystemPhase::FixedUpdate, [this](float fdt)
                    {
                      if(m_isPlaying)
                      {
                        ScriptContext ctx = makeScriptContext();
                        m_scriptSystem.physicsUpdateAll(ctx, fdt);
                      } });

    m_scheduler.add(SystemPhase::Render, [this](float dt)
                    {
                      onRender();
                      if (m_isPlaying)
                      {
                        ScriptContext ctx = makeScriptContext();
                        m_scriptSystem.renderAll(ctx, dt);
                      }
                      if (renderSceneToMainFramebuffer()) {
                            renderScene(RenderSystemContext{ m_scene, m_assetManager, m_renderer, m_window });
                      } });
  }

}
