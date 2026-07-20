# Nebula public API tiers

Headers under `Nebula/include/` are the only stable surface for games. Tier indicates who should include them.

| Header | Tier | Notes |
|--------|------|--------|
| `Nebula.h` | **game** | Default single include for Nimbus |
| `application.h` | **game** | App loop, window, input, scene, scheduler |
| `component.h` | **game** | ECS components |
| `tag_component.h` | **game** | Entity tags |
| `scene.h` | **game** | Scene / entities |
| `scene_query.h` | **game** | Find entities by tag / component |
| `sceneSerializer.h` | **game** | JSON scene load/save |
| `sceneAccess.h` | **game** | Script-facing scene view |
| `script.h` | **game** | Script interface |
| `script_Registry.h` | **game** | Register script factories |
| `scriptSystem.h` | **game** | Script lifecycle (usually via Application) |
| `eventBus.h` | **game** | Events (via application.h) |
| `eventTypes.h` | **game** | Event payload types |
| `input.h` | **game** | Keyboard/mouse (via Application) |
| `input_Actions.h` | **game** | Action mapping |
| `inputTypes.h` | **game** | Input enums |
| `frameCommands.h` | **game** | Per-frame input snapshot (`ScriptContext::input`) |
| `systemScheduler.h` | **game** | Register update/render systems |
| `clock.h` | **game** | Time |
| `assetProvider.h` | **game** | Asset path resolution |
| `assetManager.h` | **game** | Meshes/materials (via Application) |
| `asset_id.h` | **game** | Asset identifiers |
| `assetHandles.h` | **game** | Typed asset handles |
| `asset_database.h` | **internal** | Engine asset DB; prefer Application APIs |
| `math_types.h` | **game** | Vec2/3/4, Mat4 |
| `transform3D.h` | **game** | 3D transform |
| `camera3D.h` | **game** | Orbit camera math |
| `Window.h` | **game** | Window + `graphicsContext` accessor |
| `window_spec.h` | **game** | `WindowSpec` for custom window setup |
| `graphicsContext.h` | **game** | Abstract context (swap, framebuffer size) |
| `renderer_api_type.h` | **game** | `RendererAPIType` enum |
| `renderer.h` | **render-extended** | High-level draw/clear; optional beyond `Nebula.h` |
| `mesh.h` | **render-extended** | Mesh component data paths |
| `material.h` | **render-extended** | Material paths |
| `shader.h` | **render-extended** | Shader asset wrapper |
| `vertex_array.h` | **render-extended** | Mesh GPU layout (no `::create` in games) |
| `vertex_buffer.h` | **render-extended** | Vertex buffer interface |
| `index_buffer.h` | **render-extended** | Index buffer interface |
| `renderResources.h` | **render-extended** | Resource factory from `Renderer` |
| `renderSystem.h` | **internal** | Used by Application render phase |
| `imageDecoder.h` | **internal** | stb-backed; use AssetManager |
| `mesh_asset.h` | **internal** | Engine mesh asset |
| `material_asset.h` | **internal** | Engine material asset |
| `component_registry.h` | **internal** | ECS registry detail |

### Tier meanings

- **game** — Safe in `Nimbus/` via `#include <Nebula.h>` or individual headers.
- **render-extended** — Tools or custom render code; still no GLFW/GLAD/`openGL_*` in game `.cpp`.
- **internal** — Engine implementation; do not include from games (may change without notice).
