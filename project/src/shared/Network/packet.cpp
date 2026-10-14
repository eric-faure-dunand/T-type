#include <string>
#include <utility>

#include "packet.hpp"
#include "Error.hpp"

namespace r_type {

static std::string Hex(std::uint8_t code) {
    static const char digits[] = "0123456789ABCDEF";
    return std::string("0x") + digits[code >> 4] + digits[code & 0x0F];
}

static void WriteU16(std::vector<std::uint8_t>& out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>(value >> 8));
    out.push_back(static_cast<std::uint8_t>(value));
}

static void WriteU32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>(value >> 24));
    out.push_back(static_cast<std::uint8_t>(value >> 16));
    out.push_back(static_cast<std::uint8_t>(value >> 8));
    out.push_back(static_cast<std::uint8_t>(value));
}

static std::uint16_t ReadU16(std::span<const std::uint8_t> data, std::size_t offset) {
    return static_cast<std::uint16_t>((data[offset] << 8) | data[offset + 1]);
}

static std::uint32_t ReadU32(std::span<const std::uint8_t> data, std::size_t offset) {
    return (static_cast<std::uint32_t>(data[offset]) << 24)
        | (static_cast<std::uint32_t>(data[offset + 1]) << 16)
        | (static_cast<std::uint32_t>(data[offset + 2]) << 8)
        | static_cast<std::uint32_t>(data[offset + 3]);
}

static void WriteHeader(std::vector<std::uint8_t>& out, const Header& header) {
    WriteU16(out, header.magic);
    out.push_back(header.command);
    WriteU32(out, header.sequenceNumber);
    WriteU32(out, header.sequenceLength);
    WriteU16(out, header.size);
}

static Header ReadHeader(std::span<const std::uint8_t> data) {
    Header header;
    header.magic = ReadU16(data, 0);
    header.command = data[2];
    header.sequenceNumber = ReadU32(data, 3);
    header.sequenceLength = ReadU32(data, 7);
    header.size = ReadU16(data, 11);
    return header;
}

static void WriteArg(std::vector<std::uint8_t>& out, ArgType type, const Arg& arg, std::uint8_t command, std::size_t index) {
    switch (type) {
        case ArgType::U8:
            if (const auto *value = std::get_if<std::uint8_t>(&arg)) {
                out.push_back(*value);
                return;
            }
            break;
        case ArgType::U32:
            if (const auto *value = std::get_if<std::uint32_t>(&arg)) {
                WriteU32(out, *value);
                return;
            }
            break;
        case ArgType::I32:
            if (const auto *value = std::get_if<std::int32_t>(&arg)) {
                WriteU32(out, static_cast<std::uint32_t>(*value));
                return;
            }
            break;
    }
    throw Error("Packet: argument " + std::to_string(index) + " of command " + Hex(command) + " does not match its schema type.");
}

static Arg ReadArg(std::span<const std::uint8_t> data, std::size_t offset, ArgType type) {
    switch (type) {
        case ArgType::U8:
            return data[offset];
        case ArgType::U32:
            return ReadU32(data, offset);
        case ArgType::I32:
            return static_cast<std::int32_t>(ReadU32(data, offset));
    }
    return std::uint8_t{0};
}

std::vector<std::uint8_t> EncodePacket(Command command, const ArgsList& argsList, Role role) {
    const std::uint8_t code = static_cast<std::uint8_t>(command);
    const CommandInfo *info = FindCommand(code);

    if (info == nullptr || info->reserved)
        throw Error("Packet: command " + Hex(code) + " is unknown or reserved.");
    if (!CanSend(info->direction, role))
        throw Error("Packet: command " + Hex(code) + " cannot be sent in this direction.");
    if (argsList.size() != 1)
        throw Error("Packet: command " + Hex(code) + " must carry exactly one record.");

    const Args& args = argsList.front();
    if (args.size() != info->schema.size())
        throw Error("Packet: command " + Hex(code) + " expects " + std::to_string(info->schema.size())
            + " arguments, got " + std::to_string(args.size()) + ".");

    Header header;
    header.command = code;
    header.size = static_cast<std::uint16_t>(SchemaSize(info->schema));

    std::vector<std::uint8_t> datagram;
    datagram.reserve(HEADER_SIZE + header.size);
    WriteHeader(datagram, header);
    for (std::size_t i = 0; i < args.size(); i++)
        WriteArg(datagram, info->schema[i], args[i], code, i);
    return datagram;
}

static DecodeResult Reject(NetError error, std::uint8_t rejectedCommand) {
    DecodeResult result;
    result.error = error;
    result.rejectedCommand = rejectedCommand;
    return result;
}

DecodeResult DecodePacket(std::span<const std::uint8_t> datagram, Role role) {
    if (datagram.size() < HEADER_SIZE)
        return Reject(NetError::DatagramTooShort, 0);

    const Header header = ReadHeader(datagram);
    if (header.magic != MAGIC_NUMBER)
        return Reject(NetError::BadMagic, 0);
    if (datagram.size() > MAX_DATAGRAM_SIZE)
        return Reject(NetError::DatagramTooLong, header.command);
    if (header.size != datagram.size() - HEADER_SIZE)
        return Reject(NetError::SizeMismatch, header.command);
    if (header.sequenceNumber != FRAGMENT_INDEX || header.sequenceLength != FRAGMENT_COUNT)
        return Reject(NetError::FragmentationUnsupported, header.command);

    const CommandInfo *info = FindCommand(header.command);
    if (info == nullptr)
        return Reject(NetError::UnknownCommand, header.command);
    if (info->reserved)
        return Reject(NetError::ReservedCommand, header.command);
    if (!CanReceive(info->direction, role))
        return Reject(NetError::WrongDirection, header.command);
    if (header.size != SchemaSize(info->schema))
        return Reject(NetError::BadPayloadSize, header.command);

    Args args;
    args.reserve(info->schema.size());
    std::size_t offset = HEADER_SIZE;
    for (const ArgType type : info->schema) {
        args.push_back(ReadArg(datagram, offset, type));
        offset += ArgSize(type);
    }

    DecodeResult result;
    result.ok = true;
    result.command = static_cast<Command>(header.command);
    result.args.push_back(std::move(args));
    return result;
}

}
