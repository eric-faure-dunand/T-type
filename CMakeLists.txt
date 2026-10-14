cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_SOURCE_DIR}")
endforeach()

## dependencies

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-38-2
    DOWNLOAD_ONLY YES
)

set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include")
if(NOT EXISTS "${ASIO_INCLUDE_DIR}/asio.hpp")
    message(FATAL_ERROR "asio introuvable dans: ${ASIO_INCLUDE_DIR} (asio_SOURCE_DIR='${asio_SOURCE_DIR}')")
endif()
message(STATUS "asio include: ${ASIO_INCLUDE_DIR}")

if(NOT TARGET asio)
    find_package(Threads REQUIRED)
    add_library(asio INTERFACE)
    target_include_directories(asio SYSTEM INTERFACE "${ASIO_INCLUDE_DIR}")
    target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
    target_link_libraries(asio INTERFACE Threads::Threads)
endif()

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
        project/include/server/Core
        project/include/shared/Error
        project/include/shared/Network
        project/include/shared/Printer
)

target_link_libraries(r-type_server PRIVATE asio)

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