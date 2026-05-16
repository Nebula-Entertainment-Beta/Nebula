/**
 * @file application.h
 * @brief Engine **application shell**: window, input, action mapping, scene, and a default frame loop.
 *
 * **Owns:** `Window`, `Input`, `ActionMapping`, `Scene` (not `ScriptRegistry` — games keep their own
 * registry and wire `ScriptComponent` names at startup).
 *
 * Subclass and override `onUpdate` / `onRender`; call `run()` after `Window` is valid. `run()` is
 * single-shot (`m_hasRun` prevents re-entry).
 */
#pragma once

#include <string>

#include "Window.h"
#include "input.h"
#include "input_Actions.h"
#include "renderer.h"
#include "scene.h"
#include "clock.h"

namespace Nebula
{

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
        const ActionMapping &getActionMapping() const { return m_actionMapping; }
        ActionMapping &getActionMapping() { return m_actionMapping; }

        Scene &getScene() { return m_scene; }
        const Scene &getScene() const { return m_scene; }

    private:
        Window m_window;
        Input m_input;
        ActionMapping m_actionMapping;
        bool m_hasRun = false;
        bool m_rendererInitialized = false;
        uint32_t m_width;
        uint32_t m_height;
        std::string m_title;
        RendererAPIType m_rendererAPI = RendererAPIType::OpenGL;
        Scene m_scene;
        clock m_clock;
        Renderer m_renderer;
    };
}