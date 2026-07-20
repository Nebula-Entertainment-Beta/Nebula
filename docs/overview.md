# Nebula

Nebula is a small **C++ game engine** used by the club to learn graphics and engine architecture. It uses **GLFW**, **GLAD**, and an **OpenGL 4.1 core** context (see `Window.cpp`; forward-compatible on macOS), with a thin abstraction layer (`Renderer`, `Shader`, buffers, textures) so gameplay code does not depend directly on raw `gl*` calls.

## Layout

| Area | Role |
|------|------|
| `Nebula/include/` | Public entry headers (e.g. `Nebula.h`, `Window.h`) |
| `Nebula/src/` | Core runtime (e.g. `Window`) |
| `Nebula/Application/` | `Application` run loop, `Input`, `ActionMapping`, `Scene`, scripts (plus intentional stubs like `timestep`) |
| `Nebula/Math/` | **3D** transform + orbit camera implemented; **2D** camera/transform stubs kept for a future 2D pipeline |
| `Nebula/render_pipeline/interface/` | API-agnostic render types (`Renderer`, `VertexArray`, …) |
| `Nebula/render_pipeline/openGL/` | OpenGL implementations of those interfaces |
| `Nebula/render_pipeline/vulkan/` | Vulkan factory stubs (backend-ready, not required to run today) |
| `NebulaEditor/` | Qt 6 editor shell (docks + embedded GLFW Scene View) |

Third-party code under `Nebula/vendor/` is **excluded** from this manual. Game projects (e.g. under `Nimbus/`) are also **excluded** — this HTML manual documents the **engine** only.

## Suggested reading order (beginners)

1. **`Window`** — how the OpenGL context appears.
2. **`Renderer` / `RenderAPI`** — after a valid `Window`: `Renderer::init`, per frame `clear` → draw → `Window::swapBuffers` (and `pollEvents` is driven from `Application::run`).
3. **`VertexBuffer` + layout → `VertexArray` → `IndexBuffer`** — how mesh data reaches the GPU.
4. **`Shader`** — vertex + fragment programs.
5. **`Texture`** — images for sprites (with shaders that sample them).
6. **`Math/`** — read the file comments when you add cameras or world/view matrices.

Game code that links Nebula is documented in that project’s own docs, not here.

## Building docs

After configuring CMake, build the `NebulaDocs` target, then open `build/docs/html/index.html` in a browser. See [docs/README.md](README.md) for prerequisites and comment conventions.

## Source catalog (related pages)

In the generated HTML, open **Related Pages → Nebula source catalog** for a guided tour: module boundaries, frame order, why the render interface is split from OpenGL, and ECS/scene JSON notes. It complements the **File List**, which lists every engine translation unit.
