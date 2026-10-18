# Include root: code does #include "shared/ECS/Registry.hpp"
set(RTYPE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ECS core: header-only for now, no Raylib, no Asio.
# When src/shared/Systems/*.cpp appears, add an `r_type_shared` STATIC library
# (linking r_type_ecs) used by the server, the client and the demo.
add_library(r_type_ecs INTERFACE)
add_library(r_type::ecs ALIAS r_type_ecs)
target_include_directories(r_type_ecs INTERFACE ${RTYPE_INCLUDE_DIR})
target_compile_features(r_type_ecs INTERFACE cxx_std_20)

# Fixed-timestep game loop helper (no clock, no Raylib, no Asio).
add_library(r_type_loop INTERFACE)
add_library(r_type::loop ALIAS r_type_loop)
target_include_directories(r_type_loop INTERFACE ${RTYPE_INCLUDE_DIR})
target_compile_features(r_type_loop INTERFACE cxx_std_20)

# Render module, graphics-library free: IRenderer, Sprite, TextureManager,
# renderSystem, IKeyboard. Always built, so it is testable without Raylib.
add_library(r_type_render STATIC
    src/client/Render/TextureManager.cpp
    src/client/Render/RenderSystem.cpp)
add_library(r_type::render ALIAS r_type_render)
target_link_libraries(r_type_render PUBLIC r_type::ecs)
target_compile_features(r_type_render PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(r_type_render PRIVATE /W4)
else()
    target_compile_options(r_type_render PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Raylib implementation of IRenderer / IKeyboard. Raylib is linked PRIVATE: its
# headers are only visible to src/client/Render/RaylibBackend.cpp.
# Needs the X11 / OpenGL development packages on Linux (see docs/rendering.md).
# The server must not depend on it: build with -DRTYPE_BUILD_CLIENT=OFF to skip it.
option(RTYPE_BUILD_CLIENT "Build the Raylib client modules" ON)
if(RTYPE_BUILD_CLIENT)
    CPMAddPackage(
        NAME raylib
        GITHUB_REPOSITORY raysan5/raylib
        GIT_TAG 5.5
        VERSION 5.5
        OPTIONS
            "BUILD_EXAMPLES OFF"
            "BUILD_SHARED_LIBS OFF"
    )
    add_library(r_type_raylib_backend STATIC src/client/Render/RaylibBackend.cpp)
    add_library(r_type::raylib_backend ALIAS r_type_raylib_backend)
    target_link_libraries(r_type_raylib_backend PUBLIC r_type::render PRIVATE raylib)
    if(MSVC)
        target_compile_options(r_type_raylib_backend PRIVATE /W4)
    else()
        target_compile_options(r_type_raylib_backend PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    # Visual check of the render module: ./build/demo/render_demo [display_fps] [image]
    add_executable(render_demo src/demo/render_demo.cpp)
    target_link_libraries(render_demo PRIVATE r_type::raylib_backend r_type::loop)
    set_target_properties(render_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/demo")
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)
        set_target_properties(render_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/demo")
    endforeach()
    if(MSVC)
        target_compile_options(render_demo PRIVATE /W4)
    else()
        target_compile_options(render_demo PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

option(RTYPE_BUILD_TESTS "Build the unit tests" ON)
if(RTYPE_BUILD_TESTS)
    add_subdirectory(tests)
endif()