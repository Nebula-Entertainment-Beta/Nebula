#pragma once

namespace Nebula
{
    /** Which graphics API factories and `Renderer::init` should use. */
    enum class RendererAPIType
    {
        None = 0,
        OpenGL,
    };
}
