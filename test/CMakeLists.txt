cmake_minimum_required(VERSION 3.20)
project(easygl-test LANGUAGES CXX C)

include(FetchContent)

# ── GLFW ──
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ── Glad (2.x) ──
FetchContent_Declare(glad-src
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad-src)
glad_add_library(glad STATIC API gl:core=4.6)

# ── easygl (local working tree) ──
FetchContent_Declare(easygl
    SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
FetchContent_MakeAvailable(easygl)

# ── test ──
add_executable(easygl-test main.cpp)
target_link_libraries(easygl-test PRIVATE easygl::easygl)
