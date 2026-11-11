/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureTransport.cpp
*/

#include "Network/SecureTransport.hpp"
#include <limits>
#include <stdexcept>

namespace r_type {

SecureTransport::SecureTransport(
    const KeyDerivation::SessionKeys& keys, Role role):
    _sender(role == Role::Client ? keys.clientToServer : keys.serverToClient),
    _receiver(role == Role::Client ? keys.serverToClient : keys.clientToServer)
{
}

SecureTransport::SequenceBytes SecureTransport::writeSequence(
    std::uint64_t sequence)
{
    SequenceBytes bytes{};

    for (std::size_t i = 0; i < SEQUENCE_SIZE; ++i) {
        bytes[SEQUENCE_SIZE - i - 1] = static_cast<std::uint8_t>(sequence);
        sequence >>= 8;
    }
    return bytes;
}

std::uint64_t SecureTransport::readSequence(
    std::span<const std::uint8_t> datagram)
{
    std::uint64_t sequence = 0;

    for (std::size_t i = 0; i < SEQUENCE_SIZE; ++i)
        sequence = (sequence << 8) | datagram[i];
    return sequence;
}

std::vector<std::uint8_t> SecureTransport::encode(
    std::span<const std::uint8_t> datagram)
{
    if (_sendSequence == 0)
        throw std::overflow_error("Secure transport sequence exhausted");

    if (datagram.size() > MAX_PACKET_SIZE - SEQUENCE_SIZE)
        throw std::length_error("Secure transport packet is too large");

    const auto sequence = writeSequence(_sendSequence);
    const auto encrypted = _sender.encode(datagram, sequence);

    if (encrypted.size() > MAX_PACKET_SIZE - SEQUENCE_SIZE)
        throw std::length_error("Encrypted transport packet is too large");

    std::vector<std::uint8_t> packet(sequence.begin(), sequence.end());

    packet.insert(packet.end(), encrypted.begin(), encrypted.end());
    ++_sendSequence;
    return packet;
}

std::vector<std::uint8_t> SecureTransport::decode(
    std::span<const std::uint8_t> datagram)
{
    if (datagram.size() < SEQUENCE_SIZE + HEADER_SIZE
        || datagram.size() > MAX_PACKET_SIZE)
        throw std::length_error("Invalid secure transport packet size");

    const auto sequenceBytes = datagram.first(SEQUENCE_SIZE);
    const auto sequence = readSequence(sequenceBytes);

    if (!_replayWindow.accepts(sequence))
        throw std::runtime_error("Replayed secure transport packet");

    const auto encrypted = datagram.subspan(SEQUENCE_SIZE);
    const auto decrypted = _receiver.decode(encrypted, sequenceBytes);

    if (!_replayWindow.record(sequence))
        throw std::runtime_error("Cannot record secure packet sequence");

    return decrypted;
}

}
