# easygl — Tiny OpenGL 4.6 wrapper

**Single-header C++20 helper for compute-shader driven real-time simulation.**

Drop `easygl.h` into your project, link GLFW + glad, and write a `create`/`update` pair. No boilerplate. GLM is optional.

## Features

- **`start()`** — one call sets up a GLFW window + GL 4.6 context, enters the main loop
- **`context`** — named resource registry (textures, buffers, programs, pipelines)
- **Compute shaders** — `useComp()` dispatches with automatic `glMemoryBarrier`
- **Separable programs** — `glCreateShaderProgramv` + program pipelines
- **Indirect dispatch/draw** — GPU-driven via shader storage buffers
- **Uniform setters** — scalar-expanded overloads for `int`, `float`, `vec2`–`vec4`-like args, `mat4` via pointer; optionally GLM types

## Quick start — FetchContent (CMake)

```cmake
include(FetchContent)
FetchContent_Declare(easygl
    GIT_REPOSITORY https://github.com/Katsro/easygl.git
    GIT_TAG        v0.2
)
FetchContent_MakeAvailable(easygl)

target_link_libraries(myapp PRIVATE easygl::easygl)
```

GLFW 3.4 and glad (OpenGL 4.6) are fetched automatically.

### With GLM support

```cmake
set(EASYGL_USE_GLM ON CACHE BOOL "")
FetchContent_MakeAvailable(easygl)
# → glm::vec2/3/4, ivec2/3/4, mat4 uniform() overloads enabled
```

## Minimal example

```cpp
#include <easygl.h>
using namespace easygl;

void create(context &ctx) {
    ctx.imge("tex", 1024, 1024, RGBA16F);
    ctx.prog("cs", COMP, "my.comp");
    ctx.prog("vs", VERT, "quad.vert");
    ctx.prog("fs", FRAG, "shade.frag");
    ctx.pipe("pipe", "vs", "fs");
}

void update(context &ctx) {
    ctx.useImge("tex", 0, READ);
    ctx.uniform("cs", "dt", 0.016f);
    ctx.useComp("cs", 64, 64, 1);
    ctx.useDraw("pipe", STRIP, 0, 4, 1);
}

auto main() -> int {
    start("demo", create, update);
}
```

## Example — EM field simulation

`examples/emfield/` implements a **2D FDTD electromagnetic field simulation**:

- `.r` = Ez (electric) → warm red/green
- `.b`/`.a` = Hx/Hy (magnetic) → cool blue/cyan
- Hold **left mouse button** to inject an oscillating EM source

Build it:
```bash
cmake -B build -DEASYGL_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/emfield/emfield   # or emfield.exe on Windows
```

## Dependencies

| Library | Role                        | Auto-fetched? |
|---------|-----------------------------|:---:|
| GLFW    | Window + input + GL context | ✓  |
| glad    | GL 4.6 core loader          | ✓  |
| GLM     | Maths (optional)            | opt-in |

## License

MIT
