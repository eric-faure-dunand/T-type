/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecurePacket.cpp
*/

#include "Network/SecurePacket.hpp"
#include <stdexcept>

namespace r_type {

namespace {
constexpr std::size_t maxPayloadSize = 1159;
constexpr std::uint8_t rawFlag = 0;
constexpr std::uint8_t compressedFlag = 1;
}

SecurePacket::SecurePacket(const Crypter::Key& key) : _crypter(key) {}

std::string SecurePacket::encode(const std::string& payload, const std::string& aad) const {
    if (payload.size() > maxPayloadSize)
        throw std::length_error("Payload exceeds secure packet limit");

    const std::string compressed = _compressor.compress(payload);
    const bool useCompression = compressed.size() < payload.size();

    std::string data;
    data.reserve(1 + payload.size());
    data.push_back(static_cast<char>(useCompression ? compressedFlag : rawFlag));
    data += useCompression ? compressed : payload;

    const std::string encrypted = _crypter.encrypt(data, aad);
    if (encrypted.size() > maxPayloadSize)
        throw std::length_error("Encrypted payload exceeds packet limit");

    return encrypted;
}

std::string SecurePacket::decode(const std::string& payload, const std::string& aad) const {
    const std::string decrypted = _crypter.decrypt(payload, aad);
    if (decrypted.empty())
        throw std::runtime_error("Missing compression flag");

    const std::uint8_t flag = static_cast<std::uint8_t>(decrypted[0]);
    const std::string data = decrypted.substr(1);

    if (flag == rawFlag)
        return data;
    if (flag == compressedFlag)
        return _compressor.decompress(data);

    throw std::runtime_error("Invalid compression flag");
}

}