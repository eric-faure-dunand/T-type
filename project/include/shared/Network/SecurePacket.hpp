/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecurePacket.hpp
*/

#ifndef SECURE_PACKET_HPP
#define SECURE_PACKET_HPP

#include <cstdint>
#include <string>
#include <cstddef>

#include "Compression/Compressor.hpp"
#include "Encryption/Crypter.hpp"

namespace r_type {

class SecurePacket {
public:
    explicit SecurePacket(const Crypter::Key& key);

    std::string encode(const std::string& payload, const std::string& aad = "") const;
    std::string decode(const std::string& payload, const std::string& aad = "") const;

    std::size_t encodedSize(const std::string& payload) const;

private:
    Compressor _compressor;
    Crypter _crypter;
};

}

#endif