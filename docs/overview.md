# Nebula

Nebula is a small **C++ game engine** used by the club to learn graphics and engine architecture. It uses **GLFW**, **GLAD**, and **OpenGL 3.3+ core**, with a thin abstraction layer (`Renderer`, `Shader`, buffers, textures) so gameplay code does not depend directly on raw `gl*` calls.

## Layout

| Area | Role |
|------|------|
| `Nebula/include/` | Public entry headers (e.g. `Nebula.h`, `Window.h`) |
| `Nebula/src/` | Core runtime (e.g. `Window`) |
| `Nebula/Application/` | App loop, input, timestep (placeholders growing into real code) |
| `Nebula/Math/` | **Cameras and transforms** (headers documented; implementations added when a game needs them) |
| `Nebula/render_pipeline/interface/` | API-agnostic render types (`Renderer`, `VertexArray`, …) |
| `Nebula/render_pipeline/openGL/` | OpenGL implementations of those interfaces |
| `Flappy_Bird/` | Sample game sources (also listed in the manual as usage examples) |

Third-party code under `Nebula/vendor/` is **excluded** from this manual.

## Suggested reading order (beginners)

1. **`Window`** — how the OpenGL context appears.
2. **`Renderer` / `RenderAPI`** — frame flow: `clear` → draw → `Window::update`.
3. **`VertexBuffer` + layout → `VertexArray` → `IndexBuffer`** — how mesh data reaches the GPU.
4. **`Shader`** — vertex + fragment programs.
5. **`Texture`** — images for sprites (with shaders that sample them).
6. **`Math/`** — read the file comments when you add cameras or world/view matrices.

## Building docs

After configuring CMake, build the `NebulaDocs` target, then open `build/docs/html/index.html` in a browser. See [docs/README.md](README.md) for prerequisites and comment conventions.
