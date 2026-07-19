#include <easygl.h>

using namespace easygl;

void create(context &ctx) {
    ctx.imge("data0", 1024, 1024, RGBA16F);
    ctx.imge("data1", 1024, 1024, RGBA16F);

    ctx.prog("c0",   COMP, "examples/c0.comp");
    ctx.prog("c1",   COMP, "examples/c1.comp");
    ctx.prog("init", COMP, "examples/init.comp");
    ctx.useImge("data0", 0, WRITE);
    ctx.useImge("data1", 1, WRITE);
    ctx.useComp("init", 64, 64, 1);

    ctx.prog("dv", VERT, "examples/dv.vert");
    ctx.prog("df", FRAG, "examples/df.frag");
    ctx.pipe("d", "dv", "df");
}

void update(context &ctx) {
    static double t = 0.0;

    for (int i = 0; i < 16; i++) {
        ctx.useImge("data0", 0, READ);
        ctx.useImge("data1", 1, WRITE);
        ctx.useComp("c0", 64, 64, 1);

        ctx.useImge("data1", 0, READ);
        ctx.useImge("data0", 1, WRITE);

        if (glfwGetMouseButton(ctx.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mx, my;
            glfwGetCursorPos(ctx.window, &mx, &my);
            ctx.uniform("c1", "srcPos", int(mx), 1024 - int(my));
            ctx.uniform("c1", "srcVal", float(std::sin(t * 18.85) * 2.0));
            ctx.uniform("c1", "srcActive", 1.0f);
        } else {
            ctx.uniform("c1", "srcActive", 0.0f);
        }

        ctx.useComp("c1", 64, 64, 1);
        t += 1.0 / 60.0 / 16.0;
    }

    ctx.useImge("data0", 0, READ);
    ctx.useDraw("d", STRIP, 0, 4, 1);
}

auto main() -> int {
    start("EM Field Simulation", create, update, false, 1024, 1024);
    return 0;
}
