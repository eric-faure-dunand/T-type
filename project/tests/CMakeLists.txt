# enable_testing() must be called in the top-level CMakeLists.txt.
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(ecs_tests
    shared/ECS/test_entity.cpp
    shared/ECS/test_sparse_array.cpp
    shared/ECS/test_registry.cpp)
target_link_libraries(ecs_tests PRIVATE r_type::ecs GTest::gtest_main)
if(MSVC)
    target_compile_options(ecs_tests PRIVATE /W4)
else()
    target_compile_options(ecs_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
gtest_discover_tests(ecs_tests)