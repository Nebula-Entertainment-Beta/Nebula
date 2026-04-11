/**
 * @file application.h
 * @brief Placeholder for a future `Application` type (game loop + window lifetime).
 *
 * **Why this exists:** Many engines hide `while (!window.shouldClose())` inside an `Application::Run()`
 * so games only implement callbacks like `OnUpdate` and `OnRender`. Nebula will grow toward that
 * pattern; for now the loop often lives in `main.cpp`.
 */
#pragma once
