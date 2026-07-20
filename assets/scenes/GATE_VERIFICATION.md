# Gate verification (Qt / PhysX / FMOD plan)

## Automated

- [ ] `ctest --test-dir build -C Debug` (`NebulaSmokeTests`)
- [ ] CI workflow `.github/workflows/ci.yml` (Windows + Linux)

## PhysX

- [ ] `Nimbus` play: grounded jump, bounce, wind, enemy hits
- [ ] Default physics factory is PhysX (or null stub), not a custom world

## Qt editor

- [ ] Create / open / save / play / stop in `NimbusEditor`
- [ ] Docks: Hierarchy, Inspector, Console, Assets, Node Graph, FPS
- [ ] W/E/R gizmos drag in Scene View (GLFW window)

## Vertical slice

- [ ] First launch builds/loads vertical slice (saved as `scenes/vertical_slice.json` under assets when the game creates it)
- [ ] Or: Editor → New Scene → Vertical Slice
- [ ] Multi-wave → checkpoint → goal → fail/retry

## Visuals / audio / nodes

- [ ] Environment entity drives clear color / fog / light (Inspector)
- [ ] Audio stub logs `hit` / `jump` / `objective` (or FMOD if configured)
- [ ] Goal fires `graphs/objective_complete.json` → `CompleteObjective`
