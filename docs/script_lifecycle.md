# Script lifecycle and rebuild workflow

## Lifecycle hooks

Bound scripts receive hooks in this order:

1. `onCreate`
2. `onEnable`
3. `onUpdate` (every frame while playing)
4. `onPhysicsUpdate` (fixed step while playing)
5. `onRender` (every frame while playing, before scene draw)
6. `onDisable`
7. `onDestroy`

Iteration order is deterministic: ascending entity id, then generation.

## Edit mode vs Play mode

- **Edit mode:** scripts are *bound* when a scene loads, but lifecycle hooks do **not** run.
- **Play:** `activateScripts()` runs `onCreate` / `onEnable` for all bound instances.
- **Stop:** `onDisable` / `onDestroy` run, the edit-mode scene snapshot is restored, and scripts are rebound without activation.

Standalone game hosts should call `setPlaying(true)` before `onStartup()` so activation happens at launch.

## Rebuild / rebind loop

Gameplay may call `ctx.requestScriptRebuild()` after spawning entities with new `ScriptComponent`s. On the next `PostUpdate`, the engine binds any missing script instances. Newly bound scripts activate immediately only while `isPlaying()` is true.

There is no DLL hot-reload. The supported iteration loop is: edit C++ → rebuild → relaunch / Play again.
