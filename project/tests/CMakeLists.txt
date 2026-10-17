# GoogleTest through CPM (same package manager as the rest of the project).
# enable_testing() and include(cmake/CPM.cmake) are done in the root CMakeLists.txt.
CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.14.0
    VERSION 1.14.0
    OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)
include(GoogleTest)

# The root CMakeLists.txt sends every executable to the repository root:
# keep the test binaries inside the build directory instead.
set(RTYPE_TESTS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/tests")

function(rtype_add_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE GTest::gtest_main)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${RTYPE_TESTS_OUTPUT_DIR}")
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)
        set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${RTYPE_TESTS_OUTPUT_DIR}")
    endforeach()
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    gtest_discover_tests(${target})
endfunction()

rtype_add_test(ecs_tests
    shared/ECS/test_entity.cpp
    shared/ECS/test_sparse_array.cpp
    shared/ECS/test_registry.cpp
    shared/ECS/test_view.cpp
    shared/ECS/test_system_manager.cpp)
target_link_libraries(ecs_tests PRIVATE r_type::ecs)

rtype_add_test(loop_tests shared/Loop/test_fixed_timestep.cpp)
target_link_libraries(loop_tests PRIVATE r_type::loop r_type::ecs)