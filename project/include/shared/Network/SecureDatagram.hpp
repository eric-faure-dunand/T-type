/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureDatagram.hpp
*/

#ifndef SECURE_DATAGRAM_HPP
    #define SECURE_DATAGRAM_HPP

    #include "Network/SecurePacket.hpp"
    #include "Network/packet.hpp"
    #include <cstdint>
    #include <span>
    #include <vector>

namespace r_type {

class SecureDatagram {
public:
    explicit SecureDatagram(const Crypter::Key& key);

    std::vector<std::uint8_t> encode(
        std::span<const std::uint8_t> datagram,
        std::span<const std::uint8_t> extraAAD = {}) const;

    std::vector<std::uint8_t> decode(
        std::span<const std::uint8_t> datagram,
        std::span<const std::uint8_t> extraAAD = {}) const;

private:
    SecurePacket _packet;
};

}

#endif
