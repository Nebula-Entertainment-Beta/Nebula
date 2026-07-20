# Nebula
Coog Interactive's Experimental Game Engine

## Building
Nebula currently uses the CMake build system, but it may be changed out for something else in the future. Building requires cmake to be installed.
```
cmake -B build
cd build
make
```

## Documentation
Install [Doxygen](https://www.doxygen.nl/), then from the build directory:

```bash
cmake --build build --target NebulaDocs
```

Open `build/docs/html/index.html`. Conventions and details are in [docs/README.md](docs/README.md).

## Windows Setup and Run

These steps assume you are using PowerShell from the project root.

### Prerequisites
- Visual Studio 2022 with the **Desktop development with C++** workload
- CMake installed
- CMake added to your `PATH`
- Git installed

You can verify CMake is available with:

```powershell
cmake --version
```

If that fails, add `C:\Program Files\CMake\bin` to your `PATH` and reopen PowerShell.

### Clone the repository

Preferred:

```powershell
git clone --recurse-submodules <repo-url>
cd Nebula
```

If you already cloned without submodules, run:

```powershell
git submodule update --init --recursive
```

### Configure the project

PhysX is required (via vcpkg). Qt 6 is required for the editor. See [docs/MIDDLEWARE_SETUP.md](docs/MIDDLEWARE_SETUP.md).

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows
```

(Use `"Visual Studio 17 2022"` if that is what you have installed.)

### Build the project

```powershell
cmake --build build --config Debug --parallel
```

### Run the game

From the project root (Visual Studio generator; adjust `Debug`/`Release` as needed):

```powershell
.\build\Nimbus\Debug\Nimbus.exe
```

Editor:

```powershell
.\build\Nimbus\Debug\NimbusEditor.exe
```

If you `cd build` first, then run:

```powershell
.\Nimbus\Debug\Nimbus.exe
```

### Full Windows quick start

```powershell
git submodule update --init --recursive
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Debug --parallel
.\build\Nimbus\Debug\Nimbus.exe
```

More demo steps: [docs/DEMO_CHECKLIST.md](docs/DEMO_CHECKLIST.md).

## Typing Conventions
- `camelCase` for variables
- `PascalCase` for functions
- Braces must be on their own lines:
```cpp

