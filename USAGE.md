# easygl 使用指南

## 概念

easygl 是一个单头文件 OpenGL 4.6 包装器，核心思想是**命名资源注册表**：
将纹理、缓冲、着色器程序、管线全部注册为字符串名字，后续通过名字引用。

```cpp
ctx.imge("tex", 1024, 1024, RGBA16F);   // 创建
ctx.useImge("tex", 0, READ);             // 使用
```

## 生命周期

### `start()` — 入口

```cpp
easygl::start("窗口标题", create, update);
```

做三件事：
1. 创建 GLFW 窗口 + GL 4.6 core context
2. 调用 `create(ctx)` — 初始化资源
3. 进入循环：每帧调用 `update(ctx)`

全屏模式、尺寸可选：

```cpp
easygl::start("DEMO", create, update, true);             // 全屏
easygl::start("DEMO", create, update, false, 800, 600);  // 800×600
```

### `create()` — 初始化

在这里创建所有纹理、缓冲、着色器、管线。只执行一次。

### `update()` — 主循环

每一帧执行，写入 `t` 表示帧时间：

```cpp
void update(context &ctx) {
    ctx.useImge("tex", 0, WRITE);
    ctx.useComp("cs", 64, 64, 1);

    ctx.useImge("tex", 0, READ);
    ctx.useDraw("pipe", STRIP, 0, 4, 1);
}
```

## 资源类型

### 纹理 — `imge` / `useImge`

```cpp
// 创建 2D 纹理
ctx.imge("name", width, height, format);

// 格式常量
RGBA8        // 8-bit uint，默认
RGBA16F      // 半精度浮点，适合计算 + 可视化
RGBA32F      // 单精度浮点，适合高精度计算
RGBA16I/32I  // 有符号整数
RGBA16UI/32UI // 无符号整数

// 绑定到 image unit 供 compute shader 读写
ctx.useImge("name", binding, READ);   // 只读
ctx.useImge("name", binding, WRITE);  // 只写
```

### 缓冲 — `buff` / `setBuff` / `useBuff`

```cpp
// 创建缓冲
ctx.buff("name", byte_size, target);

// 目标常量
SDIB  // GL_DRAW_INDIRECT_BUFFER      — 间接绘制
SCIB  // GL_DISPATCH_INDIRECT_BUFFER  — 间接调度

// 写入数据（偏移、大小、指针）
ctx.setBuff("name", 0, sizeof(data), &data);

// 绑定到 SSBO
ctx.useBuff("name", binding);
```

### 着色器程序 — `prog` / `progSrc`

```cpp
// 从文件加载
ctx.prog("name", COMP, "path/to/shader.comp");

// 从字符串源码（不需要外部文件）
ctx.progSrc("name", COMP, R"(#version 460 core ...)");

// 着色器类型
COMP  // GL_COMPUTE_SHADER
VERT  // GL_VERTEX_SHADER
FRAG  // GL_FRAGMENT_SHADER
```

使用 `glCreateShaderProgramv` 创建**单独程序**（separable program），每个着色器阶段是一个独立 program object，而非附着到一个 program 上。

### 管线 — `pipe`

将 vert + frag 单独程序绑定到一个管线对象：

```cpp
ctx.prog("dv", VERT, "shader.vert");
ctx.prog("df", FRAG, "shader.frag");
ctx.pipe("d", "dv", "df");
```

## 计算与绘制

### Compute

```cpp
// 直接调度
ctx.useComp("name", group_x, group_y, group_z);
// 内部调用 glUseProgram + glDispatchCompute + glMemoryBarrier

// 间接调度（dispatch count 由 GPU 写入缓冲）
ctx.indComp("name", "indirect_buffer");
```

每次 dispatch 后自动插入 `GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT`，无需手动 barrier。

### 绘制

```cpp
// 直接绘制（instanced）
ctx.useDraw("pipeline_name", mode, first_vertex, vertex_count, instance_count);

// 间接绘制参数由 GPU 写入缓冲
ctx.indDraw("pipeline_name", mode, "indirect_buffer");

// 图元模式
LINE   // GL_LINE
FAN    // GL_TRIANGLE_FAN
STRIP  // GL_TRIANGLE_STRIP
```

`useDraw` 内部调用 `glUseProgram(0)` + `glBindProgramPipeline` +
`glDrawArraysInstanced`。

## uniform

所有 `uniform()` 调用通过程序名字和 uniform 名字定位。**不会缓存 uniform
location**，每次调用查询一次 `glGetUniformLocation`。

### 标量

```cpp
ctx.uniform("prog", "name", int val);
ctx.uniform("prog", "name", float val);
ctx.uniform("prog", "name", GLuint val);
```

### 向量（展开为独立参数，无需 GLM）

```cpp
ctx.uniform("prog", "name", float x, float y);          // vec2
ctx.uniform("prog", "name", float x, float y, float z); // vec3
ctx.uniform("prog", "name", float x, float y, float z, float w); // vec4

ctx.uniform("prog", "name", int x, int y);              // ivec2
ctx.uniform("prog", "name", int x, int y, int z);       // ivec3
ctx.uniform("prog", "name", int x, int y, int z, int w);// ivec4

ctx.uniform("prog", "name", float* matrix);             // mat4 (column-major)
```

### 列表

```cpp
ctx.uniform("prog", "arr", {1, 2, 3, 4});    // int[] / ivec
ctx.uniform("prog", "arr", {1.f, 2.f});       // float[]
ctx.uniform("prog", "arr", {1u, 2u, 3u});     // unsigned[]
```

### GLM 类型（可选）

定义 `EASYGL_HAVE_GLM`（或在 CMake 中 `-DEASYGL_USE_GLM=ON`）后附加：

```cpp
ctx.uniform("prog", "v", glm::vec2{...});
ctx.uniform("prog", "v", glm::vec3{...});
ctx.uniform("prog", "v", glm::vec4{...});
ctx.uniform("prog", "v", glm::ivec2{...});
ctx.uniform("prog", "v", glm::ivec3{...});
ctx.uniform("prog", "v", glm::ivec4{...});
ctx.uniform("prog", "m", glm::mat4{...});
```

## 完整最小示例

```cpp
#include <easygl.h>
using namespace easygl;

void create(context &ctx) {
    ctx.imge("tex", 512, 512, RGBA16F);
    ctx.progSrc("cs", COMP, R"(#version 460 core
        layout(local_size_x=16,local_size_y=16) in;
        layout(binding=0,rgba16f) uniform writeonly image2D tex;
        void main() {
            ivec2 p = ivec2(gl_GlobalInvocationID.xy);
            imageStore(tex, p, vec4(1,0,0,1));
        })");
    ctx.progSrc("dv", VERT, R"(#version 460 core
        out gl_PerVertex{ vec4 gl_Position; };
        void main() {
            int v = gl_VertexID;
            gl_Position = vec4((v<<1&2)-1, -(v&2)+1, 0, 1);
        })");
    ctx.progSrc("df", FRAG, R"(#version 460 core
        layout(binding=0,rgba16f) uniform readonly image2D tex;
        out vec4 c;
        void main() {
            c = imageLoad(tex, ivec2(gl_FragCoord.xy) & 511);
        })");
    ctx.pipe("d", "dv", "df");
}

void update(context &ctx) {
    ctx.useImge("tex", 0, WRITE);
    ctx.useComp("cs", 32, 32, 1);
    ctx.useImge("tex", 0, READ);
    ctx.useDraw("d", STRIP, 0, 4, 1);
}

auto main() -> int { start("Minimal", create, update); }
```

## 在项目中使用

### FetchContent

```cmake
# 先提供 GLFW + glad
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glad-src GIT_REPOSITORY https://github.com/Dav1dde/glad.git
                     GIT_TAG v2.0.8 SOURCE_SUBDIR cmake)
FetchContent_MakeAvailable(glad-src)
glad_add_library(glad STATIC API gl:core=4.6)

# 然后拉 easygl
FetchContent_Declare(easygl
    GIT_REPOSITORY https://github.com/Katsro/easygl.git
    GIT_TAG        v0.3
)
FetchContent_MakeAvailable(easygl)

target_link_libraries(myapp PRIVATE easygl::easygl)
```

### GLM 支持

```cmake
set(EASYGL_USE_GLM ON CACHE BOOL "")
```

easygl 不会自动引入 GLM——它只提供一个可选的 uniform 重载层。你仍需自行链接 GLM。

## 注意事项

- easygl 不管理 GL 资源的生命周期。窗口关闭时 `glfwTerminate` 会释放一切。
- uniform location 不做缓存——每帧大量调用 uniform 时注意性能。
- 全屏模式使用主监视器的当前分辨率。
- VAO 在 `start()` 中自动创建，无需手动操作。
- 不支持非 separable 的 program（固定管线或传统 `glAttachShader` 模式）。
