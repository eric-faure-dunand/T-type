/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureHandshake.hpp
*/

#ifndef SECURE_HANDSHAKE_HPP
    #define SECURE_HANDSHAKE_HPP

    #include "Encryption/SecureSession.hpp"
    #include <cstdint>
    #include <span>
    #include <vector>

namespace r_type {

class SecureHandshake {
public:
    enum class Type : std::uint8_t {
        Request = 1,
        Response = 2,
        Confirmation = 3,
        Ready = 4
    };

    static std::vector<std::uint8_t> encode(
        const SecureSession::Request& request);
    static std::vector<std::uint8_t> encode(
        const SecureSession::Response& response);
    static std::vector<std::uint8_t> encode(
        const SecureSession::Confirmation& confirmation);

    static Type type(std::span<const std::uint8_t> data);
    static SecureSession::Request decodeRequest(
        std::span<const std::uint8_t> data);
    static SecureSession::Response decodeResponse(
        std::span<const std::uint8_t> data);
    static SecureSession::Confirmation decodeConfirmation(
        std::span<const std::uint8_t> data);

    static std::vector<std::uint8_t> encodeReady();
    static void decodeReady(std::span<const std::uint8_t> data);

private:
    static constexpr std::uint8_t VERSION = 1;
    static constexpr std::size_t HEADER_SIZE = 4;

    static void validate(std::span<const std::uint8_t> data,
        Type expected, std::size_t size);
};

}

#endif
