# easygl — Tiny OpenGL 4.6 wrapper

**Single-header C++20 helper for compute-shader driven real-time simulation.**

Drop `easygl.h` into your project, link GLFW + glad, and write a `create`/`update` pair. No boilerplate. GLM is optional.

完整 API 文档见 [USAGE.md](USAGE.md)。

## Features

- **`start()`** — one call sets up a GLFW window + GL 4.6 context, enters the main loop
- **`context`** — named resource registry (textures, buffers, programs, pipelines)
- **Compute shaders** — `useComp()` dispatches with automatic `glMemoryBarrier`
- **Separable programs** — `glCreateShaderProgramv` + program pipelines
- **Indirect dispatch/draw** — GPU-driven via shader storage buffers
- **Uniform setters** — scalar-expanded overloads; optionally GLM types
- **`progSrc()`** — compile GLSL from a string literal, no external files needed

## Quick start — FetchContent

**easygl is a header-only interface target. It does NOT fetch GLFW or glad for you.** Provide them first:

```cmake
include(FetchContent)

# 1. Provide GLFW + glad (one-time, re-used across your project)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glad-src GIT_REPOSITORY https://github.com/Dav1dde/glad.git
                     GIT_TAG v2.0.8 SOURCE_SUBDIR cmake)
FetchContent_MakeAvailable(glad-src)
glad_add_library(glad STATIC API gl:core=4.6)

# 2. Fetch easygl (instant — no deps compiled here)
FetchContent_Declare(easygl
    GIT_REPOSITORY https://github.com/Katsro/easygl.git
    GIT_TAG        v0.3
)
FetchContent_MakeAvailable(easygl)

target_link_libraries(myapp PRIVATE easygl::easygl)
```

GLFW + glad are compiled **once** regardless of how many targets link to easygl.

### With GLM support

```cmake
set(EASYGL_USE_GLM ON CACHE BOOL "")
# glm::vec2/3/4, ivec2/3/4, mat4 uniform() overloads enabled
```

## Minimal example

```cpp
#include <easygl.h>
using namespace easygl;

void create(context &ctx) {
    ctx.imge("tex", 1024, 1024, RGBA16F);
    ctx.progSrc("cs", COMP, R"(#version 460 core ... shader source ...)");
}

void update(context &ctx) {
    ctx.useImge("tex", 0, READ);
    ctx.uniform("cs", "dt", 0.016f);
    ctx.useComp("cs", 64, 64, 1);
}

auto main() -> int { start("demo", create, update); }
```

## Example — EM field simulation

`examples/` — 2D FDTD electromagnetic field simulation with mouse interaction.

Build:
```bash
cmake -B build -G "MinGW Makefiles" -DEASYGL_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/emfield       # or emfield.exe on Windows
```
> 根据你的工具链调整 generator（Linux/macOS 默认即可，VS 用户用 `-G "Visual Studio 17 2022"`）。

- `.r` = Ez (electric) → warm red/green
- `.b`/`.a` = Hx/Hy (magnetic) → cool blue/cyan
- Hold left mouse button to inject oscillating EM source

## Dependencies

| Library | Role                        | Provided by |
|---------|-----------------------------|-------------|
| GLFW    | Window + input + GL context | **You**     |
| glad    | GL 4.6 core loader          | **You**     |
| GLM     | Maths (optional)            | opt-in      |

## License

MIT
