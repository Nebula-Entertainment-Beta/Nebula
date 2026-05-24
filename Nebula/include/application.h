/**
 * @file application.h
 * @brief Engine **application shell**: window, input, scene, scripts, and a phased frame loop.
 *
 * **Owns:** `Window`, `Input`, `ActionMapping`, `Scene`, `ScriptRegistry`, `ScriptSystem`,
 * `SystemScheduler`, and `World` (aggregates scene/assets/input/actions/scripts/frame input).
 *
 * Subclass `onStartup` (register script factories), `registerGameSystems` (gameplay systems),
 * and `onRender` (drawing). `run()` drives PreUpdate → Update → FixedUpdate → PostUpdate → Render.
 * Single-shot (`m_hasRun` prevents re-entry).
 */
#pragma once

#include <string>

#include "Window.h"
#include "input.h"
#include "scriptSystem.h"
#include "frameCommands.h"
#include "script_Registry.h"
#include "assetManager.h"
#include "systemScheduler.h"
#include "assetProvider.h"
#include "input_Actions.h"
#include "script.h"
#include "inputQuery.h"
#include "sceneAccess.h"
#include "eventBus.h"
#include "renderer.h"
#include "scriptFields.h"
#include "scene.h"
#include "clock.h"
#include "world.h"

#include <memory>

namespace Nebula
{

    class IPhysicsWorld;

    struct ApplicationSpec
    {
        std::string title = "Nebula";
        int width = 1280;
        int height = 720;
        RendererAPIType rendererAPI = RendererAPIType::OpenGL;
    };

    class Application
    {
    public:
        explicit Application(const ApplicationSpec &spec = {});
        virtual ~Application();

        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;
        Application(Application &&) = delete;
        Application &operator=(Application &&) = delete;

        void run();
        Window &getWindow() { return m_window; }
        const Window &getWindow() const { return m_window; }
        Renderer &getRenderer() { return m_renderer; }
        const Renderer &getRenderer() const { return m_renderer; }

    protected:
        Input &getInput() { return m_input; }
        const Input &getInput() const { return m_input; }
        virtual void onUpdate(float dt);
        virtual void onRender();
        virtual void onStartup();
        const ActionMapping &getActionMapping() const { return m_actionMapping; }
        ActionMapping &getActionMapping() { return m_actionMapping; }

        Scene &getScene() { return m_scene; }
        const Scene &getScene() const { return m_scene; }

        IAssetProvider &getAssets() { return m_assets; }
        const IAssetProvider &getAssets() const { return m_assets; }

        World &getWorld() { return m_world; }
        const World &getWorld() const { return m_world; }

        ScriptRegistry &getScriptRegistry() { return m_scriptRegistry; }
        ScriptContext makeScriptContext();
        SystemScheduler &getScheduler() { return m_scheduler; }

        void registerEngineSystems();
        virtual void registerGameSystems(); // games overrides

        AssetManager &getAssetManager() { return m_assetManager; }
        const AssetManager &getAssetManager() const { return m_assetManager; }

        /** Resolves `MeshRendererComponent` paths to GPU handles (requires initialized renderer). */
        void resolveSceneAssets()
        {
            if (m_rendererInitialized)
            {
                m_assetManager.resolveSceneAssets(m_scene, m_renderer.resources());
            }
        }

        EventBus &getEventBus() { return m_eventBus; }

        ScriptFieldRegistry &getScriptFieldRegistry() { return m_fieldRegistry; }
        const ScriptFieldRegistry &getScriptFieldRegistry() const { return m_fieldRegistry; }

        bool isPlaying() const { return m_isPlaying; }
        void setPlaying(bool playing) { m_isPlaying = playing; }

    private:
        Window m_window;
        Input m_input;
        ActionMapping m_actionMapping;
        bool m_hasRun = false;
        bool m_isPlaying = false;
        bool m_rendererInitialized = false;
        uint32_t m_width;
        uint32_t m_height;
        std::string m_title;
        RendererAPIType m_rendererAPI = RendererAPIType::OpenGL;
        Scene m_scene;
        clock m_clock;
        Renderer m_renderer;
        AssetManager m_assetManager;
        FileAssetProvider m_assets;
        ScriptRegistry m_scriptRegistry;
        FrameInput m_frameInput{};
        ScriptSystem m_scriptSystem;
        SystemScheduler m_scheduler;
        EventBus m_eventBus;
        ScriptFieldRegistry m_fieldRegistry;
        int m_lastFbWidth = 0;
        int m_lastFbHeight = 0;
        SceneAccess m_sceneAccess{m_scene};
        FrameInputQuery m_inputQuery{m_frameInput};
        World m_world;
        std::unique_ptr<IPhysicsWorld> m_physicsWorld;
    };
}