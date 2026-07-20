# Cloud Kingdom demo checklist

## Middleware

See [MIDDLEWARE_SETUP.md](MIDDLEWARE_SETUP.md) for Qt 6, PhysX (vcpkg), and optional FMOD.

## Build (Windows)

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Debug --parallel --target Nimbus NimbusEditor NebulaSmokeTests
```

Add `C:\Qt\6.7.3\msvc2019_64\bin` to `PATH` before running the editor.

## Standalone (Nimbus)

1. Run `Nimbus.exe` from a working directory that can resolve `assets/` (build sync copies assets into `build/assets`).
2. Confirm PhysX grounded jump, bounce pad, wind volume.
3. Clear waves → checkpoint → goal → objective log + node-graph action + audio stub lines.
4. Die / fall → retry at checkpoint.

## Editor (NimbusEditor Qt)

1. Hierarchy / Inspector / Console / Assets / Node Graph docks visible.
2. File → New / Save / Save As (Qt dialogs).
3. Select entity → edit transform; select Environment → fog/light/time of day.
4. W / E / R switch Translate / Rotate / Scale; drag RGB handles in the GLFW Scene View.
5. Play → Stop restores edit snapshot.

## Packaging

`cmake --install build --prefix dist` stages `Nimbus` + `assets/`.
