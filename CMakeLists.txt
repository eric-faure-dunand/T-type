cmake_minimum_required(VERSION 3.14)

## server

project(r-type_server CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

file(GLOB_RECURSE PROJECT_SRCS
    ${CMAKE_SOURCE_DIR}project/src/server/*.c
    ${CMAKE_SOURCE_DIR}project/src/shared/*.c
)

add_executable(r-type_server ${PROJECT_SRCS})

target_include_directories(r-type_server
    PUBLIC
        project/include/server
        project/include/shared
)

target_compile_options(r-type_server
    PRIVATE
        -Wall
        -Wextra
        -Werror
)

target_link_libraries(r-type_server
    PRIVATE
)