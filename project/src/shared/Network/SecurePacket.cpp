/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecurePacket.cpp
*/

#include "Network/SecurePacket.hpp"
#include "Network/protocol.hpp"
#include <stdexcept>

namespace r_type {

namespace {
constexpr std::size_t nonceSize = 12;
constexpr std::size_t tagSize = 16;
constexpr std::size_t flagSize = 1;
constexpr std::size_t maxEncryptedSize = MAX_DATAGRAM_SIZE - HEADER_SIZE;
constexpr std::size_t maxPlaintextSize = maxEncryptedSize - nonceSize - tagSize - flagSize;

constexpr unsigned char rawFlag = 0;
constexpr unsigned char compressedFlag = 1;
}

SecurePacket::SecurePacket(const Crypter::Key& key) : _crypter(key) {}

std::string SecurePacket::encode(const std::string& payload, const std::string& aad) const {
    if (payload.size() > MAX_PAYLOAD_SIZE)
        throw std::length_error("Payload exceeds network packet limit");

    const std::string compressed = _compressor.compress(payload);
    const bool useCompression = compressed.size() < payload.size();

    std::string data;
    data.push_back(static_cast<char>(useCompression ? compressedFlag : rawFlag));
    data += useCompression ? compressed : payload;

    if (data.size() + nonceSize + tagSize > maxEncryptedSize)
        throw std::length_error("Encrypted payload exceeds network packet limit");

    return _crypter.encrypt(data, aad);
}

std::string SecurePacket::decode(const std::string& payload, const std::string& aad) const {
    if (payload.size() > maxEncryptedSize)
        throw std::length_error("Encrypted payload exceeds network packet limit");

    const std::string decrypted = _crypter.decrypt(payload, aad);
    if (decrypted.empty())
        throw std::runtime_error("Missing compression flag");

    const unsigned char flag = static_cast<unsigned char>(decrypted[0]);
    const std::string data = decrypted.substr(1);

    std::string result;
    if (flag == rawFlag)
        result = data;
    else if (flag == compressedFlag)
        result = _compressor.decompress(data);
    else
        throw std::runtime_error("Invalid compression flag");

    if (result.size() > MAX_PAYLOAD_SIZE)
        throw std::length_error("Decompressed payload exceeds network packet limit");

    return result;
}

}
