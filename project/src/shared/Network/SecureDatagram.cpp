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
constexpr std::uint8_t secureMarker = 0x80;

std::string toString(std::span<const std::uint8_t> data) {
    return std::string(data.begin(), data.end());
}

std::vector<std::uint8_t> toBytes(const std::string& data) {
    return std::vector<std::uint8_t>(data.begin(), data.end());
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
    std::span<const std::uint8_t> datagram) const {
    if (datagram.size() < HEADER_SIZE || datagram.size() > MAX_DATAGRAM_SIZE)
        throw std::length_error("Invalid datagram size");

    const std::size_t payloadSize = datagram.size() - HEADER_SIZE;
    if (readSize(datagram) != payloadSize)
        throw std::runtime_error("Invalid payload size");
    if ((datagram[2] & secureMarker) != 0)
        throw std::runtime_error("Datagram is already marked as encrypted");

    const std::string payload = toString(datagram.subspan(HEADER_SIZE));
    const std::size_t encryptedSize = _packet.encodedSize(payload);

    if (encryptedSize > MAX_PAYLOAD_SIZE)
        throw std::length_error("Encrypted payload exceeds network limit");

    std::vector<std::uint8_t> header(datagram.begin(), datagram.begin() + HEADER_SIZE);
    header[2] |= secureMarker;
    writeSize(header, encryptedSize);

    const std::string encrypted = _packet.encode(payload, toString(header));
    const auto bytes = toBytes(encrypted);

    header.insert(header.end(), bytes.begin(), bytes.end());
    return header;
}

std::vector<std::uint8_t> SecureDatagram::decode(
    std::span<const std::uint8_t> datagram) const {
    if (datagram.size() < HEADER_SIZE || datagram.size() > MAX_DATAGRAM_SIZE)
        throw std::length_error("Invalid datagram size");

    std::vector<std::uint8_t> header(datagram.begin(), datagram.begin() + HEADER_SIZE);

    if ((header[2] & secureMarker) == 0)
        throw std::runtime_error("Datagram is not encrypted");
    if (readSize(datagram) != datagram.size() - HEADER_SIZE)
        throw std::runtime_error("Invalid encrypted payload size");

    const std::string encrypted = toString(datagram.subspan(HEADER_SIZE));
    const std::string decrypted = _packet.decode(encrypted, toString(header));

    header[2] &= static_cast<std::uint8_t>(~secureMarker);
    writeSize(header, decrypted.size());

    const auto bytes = toBytes(decrypted);
    header.insert(header.end(), bytes.begin(), bytes.end());
    return header;
}

}
