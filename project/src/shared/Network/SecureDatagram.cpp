/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureDatagram.cpp
*/

#include "Network/SecureDatagram.hpp"
#include <stdexcept>
#include <string>

namespace r_type {

namespace {
constexpr std::size_t sizeOffset = 11;

std::string toString(std::span<const std::uint8_t> data) {
    return std::string(data.begin(), data.end());
}

std::vector<std::uint8_t> toBytes(const std::string& data) {
    return std::vector<std::uint8_t>(data.begin(), data.end());
}

std::string createAAD(std::span<const std::uint8_t> header,
    std::span<const std::uint8_t> extraAAD)
{
    std::string aad = toString(header);

    aad.append(extraAAD.begin(), extraAAD.end());
    return aad;
}

void writeSize(std::vector<std::uint8_t>& data, std::size_t size) {
    data[sizeOffset] = static_cast<std::uint8_t>(size >> 8);
    data[sizeOffset + 1] = static_cast<std::uint8_t>(size);
}

std::size_t readSize(std::span<const std::uint8_t> data) {
    return (static_cast<std::size_t>(data[sizeOffset]) << 8)
        | data[sizeOffset + 1];
}
}

SecureDatagram::SecureDatagram(const Crypter::Key& key) : _packet(key) {}

std::vector<std::uint8_t> SecureDatagram::encode(
    std::span<const std::uint8_t> datagram,
    std::span<const std::uint8_t> extraAAD) const {
    if (datagram.size() < HEADER_SIZE || datagram.size() > MAX_DATAGRAM_SIZE)
        throw std::length_error("Invalid datagram size");

    const std::size_t payloadSize = datagram.size() - HEADER_SIZE;
    if (readSize(datagram) != payloadSize)
        throw std::runtime_error("Invalid payload size");

    const std::string payload = toString(datagram.subspan(HEADER_SIZE));
    const std::size_t encryptedSize = _packet.encodedSize(payload);

    if (encryptedSize > MAX_PAYLOAD_SIZE)
        throw std::length_error("Encrypted payload exceeds network limit");

    std::vector<std::uint8_t> header(datagram.begin(), datagram.begin() + HEADER_SIZE);
    writeSize(header, encryptedSize);

    const std::string encrypted = _packet.encode(
        payload, createAAD(header, extraAAD));
    const auto bytes = toBytes(encrypted);

    header.insert(header.end(), bytes.begin(), bytes.end());
    return header;
}

std::vector<std::uint8_t> SecureDatagram::decode(
    std::span<const std::uint8_t> datagram,
    std::span<const std::uint8_t> extraAAD) const {
    if (datagram.size() < HEADER_SIZE || datagram.size() > MAX_DATAGRAM_SIZE)
        throw std::length_error("Invalid datagram size");

    std::vector<std::uint8_t> header(datagram.begin(), datagram.begin() + HEADER_SIZE);

    if (readSize(datagram) != datagram.size() - HEADER_SIZE)
        throw std::runtime_error("Invalid encrypted payload size");

    const std::string encrypted = toString(datagram.subspan(HEADER_SIZE));
    const std::string decrypted = _packet.decode(
        encrypted, createAAD(header, extraAAD));

    writeSize(header, decrypted.size());

    const auto bytes = toBytes(decrypted);
    header.insert(header.end(), bytes.begin(), bytes.end());
    return header;
}

}
