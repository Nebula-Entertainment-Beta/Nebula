# Week 1-7 Gate Verification (manual)

Run from repo root with assets path `assets/` as cwd or default asset root.

## Step 3 checks performed

- [x] `assets/scenes/combat_arena.json` checked in (matches `buildDefaultScene` / `buildCombatArenaScene`)
- [x] `assets/scenes/encounter_prefab_test.json` present for Week 7 prefab encounter
- [x] Build succeeds: NebulaEditor, Nimbus, NimbusEditor

## Manual gate checklist (run in NimbusEditor)

### Gate A (Week 4)
1. File -> New Scene -> Combat Arena (or load combat_arena.json)
2. Edit entity transform in Inspector
3. Ctrl+S save
4. Play -> Stop
5. Confirm edits preserved after stop

### Week 5
1. Open Debug panel -> Combat Tuning
2. Play mode: adjust knockback / enemiesPerWave live
3. Confirm 5 enemies per wave readable in combat

### Week 6
1. File -> New Scene -> Traversal Test
2. Complete route with coyote/jump buffer and bounce pad

### Week 7
1. File -> New Scene -> Prefab Encounter Test
2. Select WaveSpawner -> waveEnemies lists enemy entities
3. Play encounter layout

## New editor features (gap closure)

- **Debug panel**: Input state overlay + Combat Tuning sliders
- **Scene View**: Transform gizmo (RGB axes) + drag-to-move on XZ
- **Inspector**: Reset button on overridden prefab fields (Transform + Script params)
