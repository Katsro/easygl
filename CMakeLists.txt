cmake_minimum_required(VERSION 3.20)
project(main LANGUAGES CXX C)

# ── C++ Standard ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# ── Dependencies ──────────────────────────────────────────────
include(FetchContent)

# ── GLFW ──
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ── Glad (2.x) ──
# glad v2 uses a glad_add_library() macro that runs the Python
# generator at build time to create the OpenGL loader source.
FetchContent_Declare(
        glad-src
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v2.0.8
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad-src)

# ponytail: compute shader needs 4.3+, keep 4.6
glad_add_library(glad STATIC API gl:core=4.6)

# ── easygl library ────────────────────────────────────────────
add_library(easygl INTERFACE)

target_link_libraries(easygl INTERFACE
        glfw
        glad
)

target_include_directories(easygl INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

# ── Executable ────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        easygl
)

# ponytail: shaders loaded via relative path, set CWD to source root
set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)


# ── Platform-specific libs ──
if (WIN32)
    target_link_libraries(easygl INTERFACE opengl32)
elseif (APPLE)
    target_link_libraries(easygl INTERFACE "-framework OpenGL")
else ()
    target_link_libraries(easygl INTERFACE GL)
endif ()
