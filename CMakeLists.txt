cmake_minimum_required(VERSION 3.20)
project(easygl VERSION 0.2.0 LANGUAGES CXX C)

# Root path preset — examples use this to resolve shader file paths
set(CMAKE_PROJECT_PATH "${CMAKE_SOURCE_DIR}" CACHE INTERNAL "")

# ── easygl interface library ──
# Consumers must provide 'glfw' and 'glad' targets before
# FetchContent_MakeAvailable(easygl).  See README for a one-liner recipe.
add_library(easygl INTERFACE)
add_library(easygl::easygl ALIAS easygl)

target_include_directories(easygl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(easygl INTERFACE glfw glad)

# ── GLM (optional) ──
option(EASYGL_USE_GLM "Enable GLM math-type uniform() overloads" OFF)
if(EASYGL_USE_GLM)
    find_package(glm REQUIRED)
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

# ── Examples (fetches its own deps) ──
option(EASYGL_BUILD_EXAMPLES "Build the example programs" OFF)
if(EASYGL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
