#include <easygl.h>

using namespace easygl;

void create(context &ctx) {
    // Two 1024×1024 RGBA16F textures for ping-pong FDTD
    // .r = Ez (electric)   .b = Hx   .a = Hy (magnetic)
    ctx.imge("data0", 1024, 1024, RGBA16F);
    ctx.imge("data1", 1024, 1024, RGBA16F);

    // Compute shaders: c0 = H-update, c1 = E-update + source
    ctx.prog("c0", COMP, "res/sdr/c0.comp");
    ctx.prog("c1", COMP, "res/sdr/c1.comp");

    // Init shader: zero textures + seed a centre pulse
    ctx.prog("init", COMP, "res/sdr/init.comp");
    ctx.useImge("data0", 0, WRITE);
    ctx.useImge("data1", 1, WRITE);
    ctx.useComp("init", 64, 64, 1);

    // Render pipeline: full-screen quad
    ctx.prog("dv", VERT, "res/sdr/dv.vert");
    ctx.prog("df", FRAG, "res/sdr/df.frag");
    ctx.pipe("d", "dv", "df");
}

void update(context &ctx) {
    static double t = 0.0;

    // ── FDTD sub-steps (16 per frame) ──
    for (int i = 0; i < 16; i++) {
        // Pass 1: H-update  (data0 → data1)
        ctx.useImge("data0", 0, READ);
        ctx.useImge("data1", 1, WRITE);
        ctx.useComp("c0", 64, 64, 1);

        // Pass 2: E-update + source  (data1 → data0)
        ctx.useImge("data1", 0, READ);
        ctx.useImge("data0", 1, WRITE);

        // ── Mouse interaction ──
        if (glfwGetMouseButton(ctx.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mx, my;
            glfwGetCursorPos(ctx.window, &mx, &my);
            // Flip Y: GLFW y=0 is top, texture y=0 is bottom
            ctx.uniform("c1", "srcPos", int(mx), 1024 - int(my));
            // Oscillating source: sine wave at ~3 Hz
            ctx.uniform("c1", "srcVal", float(std::sin(t * 18.85) * 2.0));
            ctx.uniform("c1", "srcActive", 1.0f);
        } else {
            ctx.uniform("c1", "srcActive", 0.0f);
        }

        ctx.useComp("c1", 64, 64, 1);
        t += 1.0 / 60.0 / 16.0;   // Δt per sub-step at 60 fps
    }

    // ── Render full-screen quad ──
    ctx.useImge("data0", 0, READ);
    ctx.useDraw("d", STRIP, 0, 4, 1);
}

auto main() -> int {
    start("EM Field Simulation", create, update, false, 1024, 1024);
    return 0;
}
