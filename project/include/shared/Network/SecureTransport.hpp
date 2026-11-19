/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureTransport.hpp
*/

#ifndef SECURE_TRANSPORT_HPP
    #define SECURE_TRANSPORT_HPP

    #include "Encryption/KeyDerivation.hpp"
    #include "Network/ReplayWindow.hpp"
    #include "Network/SecureDatagram.hpp"
    #include <array>
    #include <cstdint>
    #include <span>
    #include <vector>

namespace r_type {

class SecureTransport {
public:
    enum class Role {
        Client,
        Server
    };

    static constexpr std::size_t SEQUENCE_SIZE = 8;
    static constexpr std::size_t MAX_PACKET_SIZE = 1200;

    SecureTransport(const KeyDerivation::SessionKeys& keys, Role role);

    std::vector<std::uint8_t> encode(
        std::span<const std::uint8_t> datagram);
    std::vector<std::uint8_t> decode(
        std::span<const std::uint8_t> datagram);

private:
    using SequenceBytes = std::array<std::uint8_t, SEQUENCE_SIZE>;

    static SequenceBytes writeSequence(std::uint64_t sequence);
    static std::uint64_t readSequence(
        std::span<const std::uint8_t> datagram);

    SecureDatagram _sender;
    SecureDatagram _receiver;
    ReplayWindow _replayWindow;
    std::uint64_t _sendSequence = 1;
};

}

#endif
