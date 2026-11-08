#ifndef PACKET_HPP
    #define PACKET_HPP

    #include <algorithm>
    #include <cstdint>
    #include <span>
    #include <variant>
    #include <vector>
    #include <string>
    #include <utility>

    #include "protocol.hpp"
    #include "Error.hpp"

namespace r_type {

using Arg = std::variant<std::uint8_t, std::uint32_t, std::int32_t>;
using Args = std::vector<Arg>;
using ArgsList = std::vector<Args>;
using Datagram = std::vector<std::uint8_t>;

struct Header {
    std::uint16_t magic = MAGIC_NUMBER;
    std::uint8_t command = 0;
    std::uint32_t messageId = 0;
    std::uint32_t sequenceNumber = FRAGMENT_INDEX;
    std::uint32_t sequenceLength = FRAGMENT_COUNT;
    std::uint16_t size = 0;
};

struct DecodeResult {
    bool ok = false;
    Command command = Command::Error;
    std::uint32_t messageId = 0;
    std::uint32_t fragmentIndex = FRAGMENT_INDEX;
    std::uint32_t fragmentCount = FRAGMENT_COUNT;
    ArgsList args;
    NetError error = NetError::DatagramTooShort;
    std::uint8_t rejectedCommand = 0;
};

std::vector<Datagram> EncodeMessage(Command command, const ArgsList& args, Role role, std::uint32_t messageId);
DecodeResult DecodePacket(std::span<const std::uint8_t> datagram, Role role);

}

#endif
