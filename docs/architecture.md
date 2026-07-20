# Nebula architecture

This document describes **layer boundaries** and what game code (Nimbus) may depend on. CI enforces the rules in `scripts/check_nimbus_deps.sh`.

## Layers (top to bottom)

```mermaid
flowchart TB
  subgraph game [Nimbus — game]
    G[main.cpp scripts systems]
    NE[NimbusEditor host main]
  end
  subgraph editor [NebulaEditor — tools]
    EL[NebulaEditorLib Qt docks]
    EM[qt_editor_main.cpp]
  end
  subgraph public [Nebula/include — public API]
    P[Nebula.h application.h scene.h ...]
  end
  subgraph app [Nebula/Application — engine loop]
    A[Application Scene Input RenderSystem]
  end
  subgraph core [Nebula/src — platform window]
    W[Window GLFW]
    F[graphics_context_factory]
  end
  subgraph render_iface [render_pipeline/interface]
    R[Renderer RenderAPI VertexArray ...]
  end
  subgraph render_backends [render_pipeline/openGL | vulkan]
    GL[openGL_*]
    VK[vulkan_* stubs]
  end
  G --> P
  NE --> EL
  EM --> EL
  EL --> P
  P --> A
  A --> W
  A --> R
  R --> GL
  R --> VK
  W --> F
  F --> GL
  F --> VK
```

| Layer | Path | May include |
|-------|------|-------------|
| Game | `Nimbus/` | `Nebula/include` only (via `Nebula.h` or listed public headers) |
| Editor | `NebulaEditor/` | `Nebula` only — **no** game projects (`Nimbus/`, game scripts) |
| Game editor host | `Nimbus/*_editor_main.cpp` | `NebulaEditorLib` + that game's scripts/assets registration |
| Public API | `Nebula/include/` | Other public headers; no GLFW/GLAD/OpenGL |
| Application | `Nebula/Application/` | Public API + internal engine headers |
| Window / platform | `Nebula/src/` | GLFW; graphics factory picks OpenGL or Vulkan impl |
| Render interface | `Nebula/render_pipeline/interface/` | Abstract types only — **no** `openGL_*` |
| Render backends | `Nebula/render_pipeline/openGL/`, `vulkan/` | API-specific code |

## Nimbus rules

Game projects under `Nimbus/` must **not**:

- Call GLFW (`GLFW_`, `glfw`)
- Include GL loaders or GL headers (`glad`, `#include <GL/`, `gl.h`)
- Reference OpenGL backend types (`openGL_`)
- Use stb directly (`stbi_`)
- Construct GPU resources with backend static factories (`VertexArray::create`)

Use `Application`, `Scene`, `Nebula.h`, and engine services instead.

## Editor rules

`NebulaEditor/` is a **game-agnostic** tool layer:

- Builds `NebulaEditorLib` (UI, panels, play mode) and a minimal `NebulaEditor` executable with empty script/scene hooks.
- Must **not** include or link any game folder (`Nimbus/`, future games).
- Each game that wants editor support adds its own small executable (e.g. `NimbusEditor`) that links `NebulaEditorLib` and passes `ScriptRegistrar` / `NewSceneBuilder` callbacks in `main`.

## Render interface rules

Files under `Nebula/render_pipeline/interface/` must not mention OpenGL implementation types (`openGL_`). Backend selection lives in `render_backend_factory.cpp` and per-API factories under `openGL/` and `vulkan/`.

## Window vs graphics API

- **GLFW** (`Window.cpp`) — OS window, input surface, event polling.
- **Graphics context** (`src/detail/graphics_context_factory.cpp`) — OpenGL (GLAD + swap) or Vulkan (stub) based on `WindowSpec::api`.
- **Render backend** (`createRenderBackend`) — `RenderAPI` + resource factories; separate from window creation.

Adding Vulkan later should touch `graphics_context_factory.cpp`, `vulkan_GraphicsContext.*`, and `vulkan/vulkan_renderapiFactory.cpp`, not `Window.h` or game code.

## Forbidden substrings (automated)

See `scripts/check_nimbus_deps.sh` and `scripts/check_include_layers.sh`.
