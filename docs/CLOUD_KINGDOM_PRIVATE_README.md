# Cloud Kingdom Private Roadmap (12 Weeks)

Internal planning document for building Nebula into an editor-first engine while shipping a strong Cloud Kingdom vertical slice.

## Mission

By the end of 12 weeks:

- Build and play Cloud Kingdom directly inside the Nebula editor.
- Author gameplay with C++ script components attached to entities.
- Replace heavy `main.cpp` orchestration with app/scene/editor systems.
- Ship a playable "1 vs many" cartoon action vertical slice with traversal inspired by platformers.

## Product Identity

Cloud Kingdom is:

- **Combat pillar:** 1-vs-many melee arena pressure (readable telegraphs, crowd control, stagger windows).
- **Traversal pillar:** platforming momentum, aerial routes, bounce/launch moments, and recoverable mistakes.
- **Art/game feel:** cartoon-stylized readability, bold silhouettes, high clarity VFX, expressive animation timing.

Nebula should stand out as:

- **Editor-first for stylized sky worlds.**
- **Fast iteration:** play-in-editor, tweak values live, short rebuild/test loop.
- **Tooling quality:** hierarchy, inspector, gizmos, prefabs, basic node tooling.

## Non-Negotiable Technical Principles

- `window` layer only collects platform events.
- `input` layer stores per-frame state and query API.
- Controllers/systems interpret input (camera, character, cube/player).
- Runtime and editor share one engine core.
- Scene data is serializable and versionable.
- Every engine feature must be used by real Cloud Kingdom content within 7 days.

## 12-Week Roadmap

## Phase 1 - Foundation and Control Loop (Weeks 1-4)

### Week 1 - Input and camera ownership split

Goals:

- Finalize ownership boundaries (`window` capture, `input` state, controller logic).
- Remove redundant movement responsibilities from wrong layer.
- Build frame-consistent key/mouse querying model.

Deliverables:

- Input service supports held/pressed/released states and mouse delta.
- Mouse controls look rotation cleanly.
- WASD drives translation through gameplay/controller code (not window code).
- Basic debug overlay for input state while testing.

Cloud Kingdom impact:

- Reliable player feel baseline for combat and platforming.

Success checks:

- No camera jitter from inconsistent delta handling.
- No duplicated movement logic across layers.

### Week 2 - Scene/entity/component skeleton

Goals:

- Establish scene model for runtime/editor.
- Move game objects out of hardcoded flow.

Deliverables:

- Entity IDs and base components (`Transform`, `MeshRenderer`, `Camera`, `Script` placeholder).
- Scene load/save for core components.
- Update loop organized by system phases (input, simulation, render).

Cloud Kingdom impact:

- You can compose level objects and actors as data instead of one-off code.

Success checks:

- Restarting the app recreates same scene state from serialized file.

### Week 3 - ScriptComponent with C++ gameplay hooks

Goals:

- Attach C++ gameplay classes to entities.
- Define clear lifecycle functions and registration path.
- Standardize script boundaries so gameplay code uses engine services indirectly (through engine-provided context), not by opening platform/render/input libraries directly.

Deliverables:

- Script component references gameplay class type.
- Runtime invokes the full lifecycle hooks in a predictable order:
  - `onCreate`
  - `onEnable`
  - `onUpdate`
  - `onPhysicsUpdate`
  - `onRender`
  - `onDisable`
  - `onDestroy`
- Inspector-visible editable fields for script parameters (initial subset).
- Each hook receives only the parameters/components needed for that phase through engine-owned context APIs.

Cloud Kingdom impact:

- Enemies, player abilities, interactables become attachable behavior units.

Success checks:

- At least 3 behaviors attached to entities without editing core loop.

### Week 4 - Editor shell (minimum usable)

Goals:

- Use the native Qt 6 editor workflow.

Deliverables:

- Docked editor layout with: Scene View, Hierarchy, Inspector, Console.
- Entity selection/editing for transform and script fields.
- Play/Stop mode toggle preserving edit state.

Cloud Kingdom impact:

- Begin building content in-editor instead of assembling everything in `main.cpp`.

Success checks:

- Create/edit/save/play scene entirely from editor UI.

## Phase 2 - Gamefeel, Tools, and Content Throughput (Weeks 5-8)

### Week 5 - Combat prototype (1-vs-many core)

Goals:

- Build combat loop that reflects Cloud Kingdom identity.

Deliverables:

- Player light/heavy attacks with hit windows.
- Enemy spawn waves (3-8 simultaneous opponents).
- Hit reactions, knockback/stagger, invulnerability timing basics.
- Debug tuning panel (damage, cooldown, knockback, enemy aggression).

Cloud Kingdom impact:

- First real proof of "godlike hero versus crowd" fantasy.

Success checks:

- Combat remains readable at target enemy counts.

### Week 6 - Traversal prototype (platforming core)

Goals:

- Introduce platforming flavor inspired by expressive platformers.

Deliverables:

- Jump model with coyote time/jump buffer.
- Air control and fast-fall tuning.
- Bounce pad/wind lift volume prototypes.
- Traversal challenge test room in editor.

Cloud Kingdom impact:

- Movement becomes playful and not just combat locomotion.

Success checks:

- Traversal route is consistently completable and feels forgiving but skillful.

### Week 7 - Prefabs and reusable authoring

Goals:

- Improve content scale speed.

Deliverables:

- Prefab save/instantiate pipeline.
- Prefab variants/overrides for rapid enemy/object variations.
- Inspector support for nested prefab fields (minimum practical scope).

Cloud Kingdom impact:

- Faster level and encounter authoring; less duplicated setup work.

Success checks:

- Build one encounter area from prefab-driven content only.

### Week 8 - Visual identity pass (stylized sky world)

Goals:

- Lock first-pass look for cartoon cloud-fantasy world.

Deliverables:

- Atmosphere stack: sky gradient, fog tuning, directional light style.
- Cloud layer effects (fake volumetric acceptable for timeline).
- Color script presets for time-of-day mood swaps.
- VFX readability pass for hits, movement, and interactables.

Cloud Kingdom impact:

- Distinct visual signature that supports gameplay readability.

Success checks:

- Screenshots clearly read as "Cloud Kingdom" with strong silhouettes.

## Phase 3 - Editor-First Production Loop (Weeks 9-12)

### Week 9 - Script workflow hardening

Goals:

- Make C++ iteration workable for daily design.

Deliverables:

- Reliable script registration and error reporting.
- Safer play-mode transitions for script lifecycles.
- Optional quick reload strategy (if full hot reload is too risky, do stable rebuild+rebind workflow).

Cloud Kingdom impact:

- Fewer broken sessions while iterating abilities and enemy logic.

Success checks:

- Script update loop is predictable and documented.

### Week 10 - Node system foundation (authoring)

Goals:

- Begin blueprint-style tooling without destabilizing runtime.

Deliverables:

- Node graph editor MVP (events, simple math/flow nodes).
- Graph asset serialization.
- Bridge layer to runtime actions (initial subset only).

Cloud Kingdom impact:

- Designers can author simple interaction logic without core code edits.

Success checks:

- At least one encounter scriptable from node graph + components.

### Week 11 - Vertical slice assembly in editor

Goals:

- Build one polished playable section end-to-end in editor workflow.

Deliverables:

- One level chunk with traversal + arena combat + objective flow.
- Spawn waves, checkpoints, fail/retry loop.
- Minimal audio feedback pass (hits, jumps, UI prompts).

Cloud Kingdom impact:

- Demonstrates true editor-driven production capability.

Success checks:

- New test content can be assembled without touching `main.cpp`.

### Week 12 - Stabilization and demo readiness

Goals:

- Lock quality and verify claims.

Deliverables:

- Performance pass on target scenarios.
- Bug bash with issue triage list and fix sprint.
- Roadmap for next 12 weeks (multiplayer/AI depth/tools depth based on outcomes).
- Demo checklist and capture build.

Cloud Kingdom impact:

- Playable, showable slice proving engine direction and game vision.

Success checks:

- Team can author, test, and tune directly in editor reliably.

## Weekly Cadence (Do This Every Week)

- Monday: define 3 measurable goals (engine + game + tooling).
- Midweek: internal playtest with notes (feel/readability/performance).
- Friday: milestone branch tag + changelog entry + short retro.
- Sunday: plan next week based on blocked work and test findings.

## Milestone Gates

Gate A (end Week 4):

- Editor can create/save/play scenes.
- Script components attach and run.

Gate B (end Week 8):

- Combat + traversal loop playable with recognizable visual identity.

Gate C (end Week 12):

- Cloud Kingdom vertical slice developed mostly inside editor.
- `main.cpp` is minimal bootstrapping, not gameplay orchestration.

## Scope Control Rules

- If a task does not improve Cloud Kingdom in 2 weeks, defer it.
- Do not add advanced rendering features before gameplay readability is stable.
- Keep node system MVP tiny; expand only after it ships one real gameplay use case.
- Prefer completion and reliability over ambitious unfinished subsystems.

## Risks and Mitigation

- **Risk:** C++ iteration too slow.  
  **Mitigation:** strict module boundaries, incremental builds, stable rebuild+rebind pipeline.

- **Risk:** Editor complexity explodes.  
  **Mitigation:** ship minimum editor panels first; gate new tools behind real content needs.

- **Risk:** Combat and traversal compete for tuning time.  
  **Mitigation:** alternate focused polish days and keep shared movement/combat metrics.

- **Risk:** Visual effects hurt readability/performance.  
  **Mitigation:** enforce readability test scene and fixed perf budget checks weekly.

## Definition of Success (End of 12 Weeks)

- You build Cloud Kingdom content directly in Nebula editor.
- Entities get behavior by attaching C++ script components.
- The game has a clear 1-vs-many + platforming identity and cartoon style.
- Nebula has a distinct value proposition: fast editor workflow for stylized action worlds.

## Next 12-Week Preview (After This Plan)

- Expand node graph into fuller blueprint layer.
- Add deeper AI behaviors and encounter authoring tools.
- Improve asset pipeline and content packaging.
- Explore optional secondary scripting language for faster iteration loops.

