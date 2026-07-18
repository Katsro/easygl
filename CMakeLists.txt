cmake_minimum_required(VERSION 3.20)
project(easygl VERSION 0.2.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

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

# ── GLM (optional) ──
option(EASYGL_USE_GLM "Enable GLM math-type uniform() overloads" OFF)
if(EASYGL_USE_GLM)
    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glm)
endif()

# ── easygl interface library ──
add_library(easygl INTERFACE)
add_library(easygl::easygl ALIAS easygl)

target_link_libraries(easygl INTERFACE glfw glad)
target_include_directories(easygl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(EASYGL_USE_GLM)
    target_link_libraries(easygl INTERFACE glm::glm)
    target_compile_definitions(easygl INTERFACE EASYGL_HAVE_GLM)
endif()

# ── Platform OpenGL ──
if(WIN32)
    target_link_libraries(easygl INTERFACE opengl32)
elseif(APPLE)
    target_link_libraries(easygl INTERFACE "-framework OpenGL")
else()
    target_link_libraries(easygl INTERFACE GL)
endif()

# ── Examples ──
option(EASYGL_BUILD_EXAMPLES "Build the example programs" ON)
if(EASYGL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
