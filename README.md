# easygl — Tiny OpenGL 4.6 wrapper

**Single-header C++20 helper for compute-shader driven real-time simulation.**

Drop `easygl.h` into your project, link GLFW + glad (OpenGL 4.6) + GLM, and write a
`create` / `update` pair.  No boilerplate.

## Features

- **`start()`** — one call sets up a GLFW window + GL context, enters the main loop.
- **`context`** — named resource registry (textures, buffers, programs, pipelines).
- **Compute shaders** — `useComp()` dispatches with automatic `glMemoryBarrier`.
- **Separable programs** — `glCreateShaderProgramv` + program pipelines.
- **Indirect dispatch/draw** — compute and vertex by GPU-written buffer.
- **Uniform setters** — template specializations for `int`, `float`, `vec2`–`vec4`,
  `ivec2`–`ivec4`, `mat4`, `std::initializer_list`.

## Quick example

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
    ctx.useImge("tex", 1, WRITE);           // ping-pong on the same texture
    ctx.uniform("cs", "dt", 0.016f);
    ctx.useComp("cs", 64, 64, 1);           // 1024² with 16×16 workgroups
    ctx.useDraw("pipe", STRIP, 0, 4, 1);
}

auto main() -> int {
    start("demo", create, update);
}
```

## Example — EM field simulation

`src/main.cpp` + `res/sdr/*.comp` + `res/sdr/*.frag` implement a
**2D FDTD electromagnetic field simulation** with mouse interaction:

- `.r` = Ez (electric) → warm red/green
- `.b`/`.a` = Hx/Hy (magnetic) → cool blue/cyan
- Hold **left mouse button** to inject an oscillating EM source.

## Dependencies

| Library | Role                       |
|---------|----------------------------|
| GLFW    | Window + input + GL context |
| glad    | GL 4.6 core loader         |
| GLM     | Maths (`glm::vec*`, `mat4`) |

They are automatically fetched by the CMake build (FetchContent).

## Build

```bash
cmake -B build
cmake --build build
./build/main
```

## License

MIT — do whatever you want.
