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

option(RTYPE_BUILD_TESTS "Build the unit tests" ON)
if(RTYPE_BUILD_TESTS)
    add_subdirectory(tests)
endif()