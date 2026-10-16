cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)              # adapte selon ton projet
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_SOURCE_DIR}")
endforeach()

## server

project(r-type_server CXX)

file(GLOB_RECURSE PROJECT_SRCS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/project/src/server/*.cpp
    ${CMAKE_SOURCE_DIR}/project/src/shared/*.cpp
)

add_executable(r-type_server ${PROJECT_SRCS})

target_include_directories(r-type_server
    PUBLIC
        project/include/server
        project/include/server/Security
        project/include/shared/Error
        project/include/shared/Network
        project/include/shared/Printer
)

if(MSVC)
    target_compile_options(r-type_server PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(r-type_server PRIVATE -Wall -Wextra -Werror)
endif()

if(WIN32)
    target_link_libraries(r-type_server PRIVATE ws2_32 wsock32)
    target_compile_definitions(r-type_server PRIVATE
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()