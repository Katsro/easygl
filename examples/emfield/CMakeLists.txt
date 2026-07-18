add_executable(emfield main.cpp)

target_link_libraries(emfield PRIVATE easygl::easygl)

# CWD = examples/emfield/ so shader paths ("res/sdr/…") resolve correctly
set_target_properties(emfield PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
