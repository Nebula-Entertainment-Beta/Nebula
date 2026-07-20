# Gate verification (Qt / PhysX / FMOD plan)

## Automated

- [ ] `ctest --test-dir build -C Debug` (`NebulaSmokeTests`)
- [ ] CI workflow `.github/workflows/ci.yml` (Windows + Linux)

## PhysX

- [ ] `Nimbus` play: grounded jump, bounce, wind, enemy hits
- [ ] No `SimplePhysicsWorld` in default link

## Qt editor

- [ ] Create / open / save / play / stop in `NimbusEditor`
- [ ] Docks: Hierarchy, Inspector, Console, Assets, Node Graph, FPS
- [ ] W/E/R gizmos drag in Scene View (GLFW window)

## Vertical slice

- [ ] Default scene `scenes/vertical_slice.json` (or New Scene → Vertical Slice)
- [ ] Multi-wave → checkpoint → goal → fail/retry

## Visuals / audio / nodes

- [ ] Environment entity drives clear color / fog / light (Inspector)
- [ ] Audio stub logs `hit` / `jump` / `objective` (or FMOD if configured)
- [ ] Goal fires `graphs/objective_complete.json` → `CompleteObjective`
