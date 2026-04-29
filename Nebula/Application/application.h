/**
 * @file application.h
 * @brief Placeholder for a future `Application` type (game loop + window lifetime).
 *
 * **Why this exists:** Many engines hide `while (!window.shouldClose())` inside an `Application::Run()`
 * so games only implement callbacks like `OnUpdate` and `OnRender`. Nebula will grow toward that
 * pattern; for now the loop often lives in `main.cpp`.
 */
#pragma once

#include <string>

#include "Window.h"
#include "input.h"
#include "input_Actions.h"
#include "scene.h"
#include "script_Registry.h"
#include "script.h"

namespace Nebula
{

    struct ApplicationSpec
    {
        std::string title = "Nebula";
        int width = 1280;
        int height = 720;
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
        Scene m_scene;
        };
}