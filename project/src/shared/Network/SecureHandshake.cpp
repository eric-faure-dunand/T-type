/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureHandshake.cpp
*/

#include "Network/SecureHandshake.hpp"
#include <algorithm>
#include <stdexcept>

namespace r_type {

namespace {

template <std::size_t Size>
void append(std::vector<std::uint8_t>& output,
    const std::array<std::uint8_t, Size>& value)
{
    output.insert(output.end(), value.begin(), value.end());
}

template <std::size_t Size>
void read(std::span<const std::uint8_t> data, std::size_t offset,
    std::array<std::uint8_t, Size>& output)
{
    std::copy_n(data.begin() + offset, Size, output.begin());
}

std::vector<std::uint8_t> header(SecureHandshake::Type type)
{
    return {0x53, 0x48, 1, static_cast<std::uint8_t>(type)};
}

}

std::vector<std::uint8_t> SecureHandshake::encode(
    const SecureSession::Request& request)
{
    auto data = header(Type::Request);

    append(data, request.publicKey);
    append(data, request.nonce);
    return data;
}

std::vector<std::uint8_t> SecureHandshake::encode(
    const SecureSession::Response& response)
{
    auto data = header(Type::Response);

    append(data, response.publicKey);
    append(data, response.nonce);
    append(data, response.signature);
    return data;
}

std::vector<std::uint8_t> SecureHandshake::encode(
    const SecureSession::Confirmation& confirmation)
{
    auto data = header(Type::Confirmation);

    append(data, confirmation);
    return data;
}

SecureHandshake::Type SecureHandshake::type(
    std::span<const std::uint8_t> data)
{
    if (data.size() < HEADER_SIZE || data[0] != 0x53 ||
        data[1] != 0x48 || data[2] != VERSION)
        throw std::runtime_error("Invalid handshake header");

    switch (data[3]) {
        case 1: return Type::Request;
        case 2: return Type::Response;
        case 3: return Type::Confirmation;
        case 4: return Type::Ready;
        default: throw std::runtime_error("Unknown handshake type");
    }
}

void SecureHandshake::validate(
    std::span<const std::uint8_t> data,
    Type expected, std::size_t size)
{
    if (data.size() != size || type(data) != expected)
        throw std::runtime_error("Invalid handshake message");
}

SecureSession::Request SecureHandshake::decodeRequest(
    std::span<const std::uint8_t> data)
{
    SecureSession::Request request{};

    validate(data, Type::Request, HEADER_SIZE + 64);
    read(data, HEADER_SIZE, request.publicKey);
    read(data, HEADER_SIZE + 32, request.nonce);
    return request;
}

SecureSession::Response SecureHandshake::decodeResponse(
    std::span<const std::uint8_t> data)
{
    SecureSession::Response response{};

    validate(data, Type::Response, HEADER_SIZE + 128);
    read(data, HEADER_SIZE, response.publicKey);
    read(data, HEADER_SIZE + 32, response.nonce);
    read(data, HEADER_SIZE + 64, response.signature);
    return response;
}

SecureSession::Confirmation SecureHandshake::decodeConfirmation(
    std::span<const std::uint8_t> data)
{
    SecureSession::Confirmation confirmation{};

    validate(data, Type::Confirmation, HEADER_SIZE + 32);
    read(data, HEADER_SIZE, confirmation);
    return confirmation;
}

std::vector<std::uint8_t> SecureHandshake::encodeReady()
{
    return header(Type::Ready);
}

void SecureHandshake::decodeReady(std::span<const std::uint8_t> data)
{
    validate(data, Type::Ready, HEADER_SIZE);
}

}
