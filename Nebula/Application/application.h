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


namespace Nebula{

  struct ApplicationSpec{
    std::string title ="Nebula";
    int width = 1280;
    int height = 720;

  

  };

  class Application{

    explicit Application(const ApplicationSpec& spec={});
    virtual ~Application();
    Application(const Application&) = delete;
    Application& operator = (const Application&) = delete;

    void run();
    Window& getWindow() {return m_window;}
    const Window& getWindow() const { return m_window; }
protected:
    virtual void onUpdate(float dt) {}
    virtual void onRender() {}
private:
    Window m_window;




  };
}