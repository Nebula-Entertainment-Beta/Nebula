# Nebula
Coog Interactive's Experimental Game Engine

## Building
Nebula currently uses the CMake build system, but it may be changed out for something else in the future. Building requires cmake to be installed.
```
cmake -B build
cd build
make
```

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

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

### Build the project

```powershell
cmake --build build --config Debug
```

### Run the game

From the project root:

```powershell
.\build\App\Debug\Game.exe
```

If you `cd build` first, then run:

```powershell
.\App\Debug\Game.exe
```

### Full Windows quick start

```powershell
git submodule update --init --recursive
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
cd build
.\App\Debug\Game.exe
```

## Typing Conventions
- `camelCase` for variables
- `PascalCase` for functions
- Braces must be on their own lines:
```cpp

