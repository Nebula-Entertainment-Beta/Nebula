# Middleware setup (Qt / PhysX / FMOD)

## PhysX (required)

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install physx:x64-windows
```

Configure with:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Qt 6 (required for editor)

Installed via `aqtinstall` to `C:\Qt\6.7.3\msvc2019_64` (or set `CMAKE_PREFIX_PATH`).

Ensure `C:\Qt\6.7.3\msvc2019_64\bin` is on `PATH` when running `NimbusEditor.exe`.

## FMOD (optional)

1. Download FMOD Engine from https://www.fmod.com/
2. Set `FMOD_ROOT` to the API folder containing `inc/` and `lib/`
3. Reconfigure CMake (looks for FMOD and defines `NEBULA_USE_FMOD`)

Without FMOD, `createAudioService()` uses a stub that logs one-shot requests.
