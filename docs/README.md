# Nebula documentation

## Generated API manual (Doxygen)

**Prerequisite:** [Doxygen](https://www.doxygen.nl/) 1.9.x or newer on your `PATH`.

```bash
cmake -S . -B build
cmake --build build --target NebulaDocs
```

Open **`build/docs/html/index.html`** in a browser.

The Doxygen input is **`Nebula/`** (engine) and **`Flappy_Bird/src/`** (sample game). Paths matching **`*/vendor/*`** are excluded so GLFW/GLM/GLAD do not flood the manual.

## What to write in source files

Doxygen picks up special comment blocks. You do **not** need to decorate every line—start with file and class summaries, then add detail as the API stabilizes.

**File header (top of `.h` / `.cpp`):**

```cpp
/**
 * @file vertex_array.h
 * @brief Binds vertex layouts and optional index buffers for indexed draws.
 */
```

**Class / brief function:**

```cpp
/**
 * @brief GPU texture wrapper (2D, RGBA8 from client memory).
 */
class Texture { ... };

/// Binds this texture to the given texture unit (e.g. 0 for GL_TEXTURE0).
virtual void bind(uint32_t unit) const = 0;
```

**Parameters and return value:**

```cpp
/**
 * Creates a 2D texture from raw RGBA bytes.
 * @param width  Pixel width (> 0).
 * @param height Pixel height (> 0).
 * @param rgba   Pointer to width*height*4 bytes; must outlive upload if used synchronously.
 * @return Shared pointer, or nullptr on invalid arguments or upload failure.
 */
```

With **`EXTRACT_ALL = YES`**, undocumented types still appear in the generated pages so the manual stays a complete file index while comments catch up.

## Optional: disable the docs target

Configure with `-DNEBULA_BUILD_DOCS=OFF` if you do not want the `NebulaDocs` target (or CI without Doxygen).
