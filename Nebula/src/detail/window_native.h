/**
 * @file window_native.h
 * @brief Engine-internal native window access — included only from Nebula `Window` / `Input`
 *        implementation .cpp files. Not part of the game-facing API.
 */
#pragma once

namespace Nebula
{
    class Window;

    namespace detail
    {
        /** GLFW window pointer cast to void*; null if invalid. Do not use from game code. */
        void *nativeWindowHandleForInput(const Window &window);
    }
} // namespace Nebula
