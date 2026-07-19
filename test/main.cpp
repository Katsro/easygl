// Minimal reusability test: header compiles, types resolve, linking succeeds
#include <easygl.h>
auto main() -> int {
    easygl::context ctx;
    (void)ctx;
}
