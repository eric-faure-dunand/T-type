cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_SOURCE_DIR}")
endforeach()

project(r-type_server CXX)

## dependencies

include(cmake/CPM.cmake)

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

CPMAddPackage(
    NAME lz4
    GITHUB_REPOSITORY lz4/lz4
    GIT_TAG v1.10.0
    DOWNLOAD_ONLY YES
)

add_library(r_type_lz4 STATIC
    "${lz4_SOURCE_DIR}/lib/lz4.c"
)

target_include_directories(r_type_lz4 PUBLIC
    "${lz4_SOURCE_DIR}/lib"
)

# Unit tests
enable_testing()

CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.15.2
    OPTIONS
        "gtest_force_shared_crt ON"
)

CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-38-2
    DOWNLOAD_ONLY YES
)

set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/include")
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
        project/include/server/Core/Room
        project/include/shared
        project/include/shared/Error
        project/include/shared/Network
        project/include/shared/Printer
        project/include/shared/ECS
        project/include/shared/Components
        project/include/shared/Systems
        project/include/shared/Game
)

target_link_libraries(r-type_server PRIVATE asio r_type_lz4 OpenSSL::Crypto)

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

## client

enable_language(C)
CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 5.5
    VERSION 5.5
    OPTIONS
        "BUILD_EXAMPLES OFF"
        "BUILD_SHARED_LIBS OFF"
)

file(GLOB_RECURSE CLIENT_SRCS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/project/src/client/*.cpp
    ${CMAKE_SOURCE_DIR}/project/src/shared/*.cpp
)

add_executable(r-type_client ${CLIENT_SRCS})

target_include_directories(r-type_client
    PUBLIC
        project/include/client/Core
        project/include/client/Display
        project/include/client/World
        project/include/shared
        project/include/shared/Error
        project/include/shared/Network
        project/include/shared/Printer
        project/include/shared/ECS
        project/include/shared/Components
        project/include/shared/Systems
        project/include/shared/Game
)

target_link_libraries(r-type_client PRIVATE asio raylib r_type_lz4 OpenSSL::Crypto)

if(MSVC)
    target_compile_options(r-type_client PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(r-type_client PRIVATE -Wall -Wextra -Werror)
endif()

if(WIN32)
    target_link_libraries(r-type_client PRIVATE ws2_32 wsock32)
    target_compile_definitions(r-type_client PRIVATE
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    if(MSVC)
        set_target_properties(r-type_client PROPERTIES WIN32_EXECUTABLE ON)
        target_link_options(r-type_client PRIVATE /ENTRY:mainCRTStartup)
    endif()
endif()

# Compression tests
add_executable(compressor_tests
    project/tests/shared/Compression/test_compressor.cpp
    project/src/shared/Compression/Compressor.cpp
)

target_include_directories(compressor_tests PRIVATE
    project/include/shared
)

target_link_libraries(compressor_tests PRIVATE
    GTest::gtest_main
    r_type_lz4
)

include(GoogleTest)
gtest_discover_tests(compressor_tests)


# Encryption tests
add_executable(crypter_tests
    project/tests/shared/Encryption/test_crypter.cpp
    project/src/shared/Encryption/Crypter.cpp
)
target_include_directories(crypter_tests PRIVATE
    project/include/shared
)
target_link_libraries(crypter_tests PRIVATE
    GTest::gtest_main
    OpenSSL::Crypto
)
gtest_discover_tests(crypter_tests)

# Secure pipeline tests
add_executable(secure_pipeline_tests
    project/tests/shared/test_secure_pipeline.cpp
    project/src/shared/Compression/Compressor.cpp
    project/src/shared/Encryption/Crypter.cpp
)
target_include_directories(secure_pipeline_tests PRIVATE
    project/include/shared
)
target_link_libraries(secure_pipeline_tests PRIVATE
    GTest::gtest_main
    r_type_lz4
    OpenSSL::Crypto
)
gtest_discover_tests(secure_pipeline_tests)

# secure packet tests

add_executable(secure_packet_tests
    project/tests/shared/Network/test_secure_packet.cpp
    project/src/shared/Network/SecurePacket.cpp
    project/src/shared/Compression/Compressor.cpp
    project/src/shared/Encryption/Crypter.cpp
)

target_include_directories(secure_packet_tests PRIVATE project/include/shared)

target_link_libraries(secure_packet_tests PRIVATE
    GTest::gtest_main
    r_type_lz4
    OpenSSL::Crypto
)

gtest_discover_tests(secure_packet_tests)

# secure datagram test

add_executable(secure_datagram_tests
    project/tests/shared/Network/test_secure_datagram.cpp
    project/src/shared/Network/SecureDatagram.cpp
    project/src/shared/Network/SecurePacket.cpp
    project/src/shared/Compression/Compressor.cpp
    project/src/shared/Encryption/Crypter.cpp
    project/src/shared/Network/packet.cpp
    project/src/shared/Network/protocol.cpp
)

target_include_directories(secure_datagram_tests PRIVATE
    project/include/shared
    project/include/shared/Network
    project/include/shared/Error
    project/include/shared/Printer
)

target_link_libraries(secure_datagram_tests PRIVATE
    GTest::gtest_main
    r_type_lz4
    OpenSSL::Crypto
)

gtest_discover_tests(secure_datagram_tests)

# key exchange tests

add_executable(key_exchange_tests
    project/tests/shared/Encryption/test_key_exchange.cpp
    project/src/shared/Encryption/KeyExchange.cpp
)

target_include_directories(key_exchange_tests PRIVATE
    project/include/shared
)

target_link_libraries(key_exchange_tests PRIVATE
    GTest::gtest_main
    OpenSSL::Crypto
)

gtest_discover_tests(key_exchange_tests)


# Key derivation tests

add_executable(key_derivation_tests
    project/tests/shared/Encryption/test_key_derivation.cpp
    project/src/shared/Encryption/KeyDerivation.cpp
    project/src/shared/Encryption/KeyExchange.cpp
)

target_include_directories(key_derivation_tests PRIVATE
    project/include/shared
)

target_link_libraries(key_derivation_tests PRIVATE
    GTest::gtest_main
    OpenSSL::Crypto
)

gtest_discover_tests(key_derivation_tests)


# Server identity tests

add_executable(server_identity_tests
    project/tests/shared/Encryption/test_server_identity.cpp
    project/src/shared/Encryption/ServerIdentity.cpp
)

target_include_directories(server_identity_tests PRIVATE
    project/include/shared
)

target_link_libraries(server_identity_tests PRIVATE
    GTest::gtest_main
    OpenSSL::Crypto
)

gtest_discover_tests(server_identity_tests)


# Secure session tests

add_executable(secure_session_tests
    project/tests/shared/Encryption/test_secure_session.cpp
    project/src/shared/Encryption/SecureSession.cpp
    project/src/shared/Encryption/KeyExchange.cpp
    project/src/shared/Encryption/KeyDerivation.cpp
    project/src/shared/Encryption/ServerIdentity.cpp
)

target_include_directories(secure_session_tests PRIVATE
    project/include/shared
)

target_link_libraries(secure_session_tests PRIVATE
    GTest::gtest_main
    OpenSSL::Crypto
)

gtest_discover_tests(secure_session_tests)


# Replay window tests

add_executable(replay_window_tests
    project/tests/shared/Network/test_replay_window.cpp
    project/src/shared/Network/ReplayWindow.cpp
)

target_include_directories(replay_window_tests PRIVATE
    project/include/shared
)

target_link_libraries(replay_window_tests PRIVATE
    GTest::gtest_main
)

gtest_discover_tests(replay_window_tests)


# Secure transport tests

add_executable(secure_transport_tests
    project/tests/shared/Network/test_secure_transport.cpp
    project/src/shared/Network/SecureTransport.cpp
    project/src/shared/Network/SecureDatagram.cpp
    project/src/shared/Network/SecurePacket.cpp
    project/src/shared/Network/ReplayWindow.cpp
    project/src/shared/Encryption/Crypter.cpp
    project/src/shared/Compression/Compressor.cpp
    project/src/shared/Network/packet.cpp
    project/src/shared/Network/protocol.cpp
)

target_include_directories(secure_transport_tests PRIVATE
    project/include/shared
    project/include/shared/Error
    project/include/shared/Network
    project/include/shared/Printer
)

target_link_libraries(secure_transport_tests PRIVATE
    GTest::gtest_main
    OpenSSL::Crypto
    r_type_lz4
)

gtest_discover_tests(secure_transport_tests)


# Secure session manager tests

add_executable(secure_session_manager_tests
    project/tests/shared/Network/test_secure_session_manager.cpp
    project/src/shared/Network/SecureSessionManager.cpp
    project/src/shared/Network/SecureTransport.cpp
    project/src/shared/Network/SecureDatagram.cpp
    project/src/shared/Network/SecurePacket.cpp
    project/src/shared/Network/ReplayWindow.cpp
    project/src/shared/Network/packet.cpp
    project/src/shared/Network/protocol.cpp
    project/src/shared/Encryption/SecureSession.cpp
    project/src/shared/Encryption/KeyExchange.cpp
    project/src/shared/Encryption/KeyDerivation.cpp
    project/src/shared/Encryption/ServerIdentity.cpp
    project/src/shared/Encryption/Crypter.cpp
    project/src/shared/Compression/Compressor.cpp
)

target_include_directories(secure_session_manager_tests PRIVATE
    project/include/shared
    project/include/shared/Network
    project/include/shared/Error
    project/include/shared/Printer
)

target_link_libraries(secure_session_manager_tests PRIVATE
    GTest::gtest_main
    r_type_lz4
    OpenSSL::Crypto
    asio
)

gtest_discover_tests(secure_session_manager_tests)


add_executable(secure_handshake_tests
    project/tests/shared/Network/test_secure_handshake.cpp
    project/src/shared/Network/SecureHandshake.cpp
)

target_include_directories(secure_handshake_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/project/include/shared
)

target_link_libraries(secure_handshake_tests PRIVATE
    GTest::gtest_main
)

gtest_discover_tests(secure_handshake_tests)