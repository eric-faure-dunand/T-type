#include "packet.hpp"

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
    WriteU32(out, header.messageId);
    WriteU32(out, header.sequenceNumber);
    WriteU32(out, header.sequenceLength);
    WriteU16(out, header.size);
}

static Header ReadHeader(std::span<const std::uint8_t> data) {
    Header header;
    header.magic = ReadU16(data, 0);
    header.command = data[2];
    header.messageId = ReadU32(data, 3);
    header.sequenceNumber = ReadU32(data, 7);
    header.sequenceLength = ReadU32(data, 11);
    header.size = ReadU16(data, 15);
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

static Datagram EncodeFragment(const CommandInfo& info, std::uint8_t code, std::span<const Args> records, Header header) {
    header.command = code;
    header.size = static_cast<std::uint16_t>(records.size() * SchemaSize(info.schema));

    Datagram datagram;
    datagram.reserve(HEADER_SIZE + header.size);
    WriteHeader(datagram, header);
    for (const Args& args : records) {
        if (args.size() != info.schema.size())
            throw Error("Packet: command " + Hex(code) + " expects " + std::to_string(info.schema.size())
                + " arguments, got " + std::to_string(args.size()) + ".");
        for (std::size_t i = 0; i < args.size(); i++)
            WriteArg(datagram, info.schema[i], args[i], code, i);
    }
    return datagram;
}

std::vector<Datagram> EncodeMessage(Command command, const ArgsList& argsList, Role role, std::uint32_t messageId) {
    const std::uint8_t code = static_cast<std::uint8_t>(command);
    const CommandInfo *info = FindCommand(code);

    if (info == nullptr || info->reserved)
        throw Error("Packet: command " + Hex(code) + " is unknown or reserved.");
    if (!CanSend(info->direction, role))
        throw Error("Packet: command " + Hex(code) + " cannot be sent in this direction.");
    if (!info->list && argsList.size() != 1)
        throw Error("Packet: command " + Hex(code) + " must carry exactly one record.");

    const std::size_t recordSize = SchemaSize(info->schema);
    if (recordSize > MAX_PAYLOAD_SIZE)
        throw Error("Packet: command " + Hex(code) + " record of " + std::to_string(recordSize)
            + " bytes exceeds " + std::to_string(MAX_PAYLOAD_SIZE) + ".");

    const std::size_t perDatagram = recordSize == 0 ? 1 : MAX_PAYLOAD_SIZE / recordSize;
    const std::size_t count = std::max<std::size_t>(1, (argsList.size() + perDatagram - 1) / perDatagram);
    if (count > MAX_FRAGMENTS)
        throw Error("Packet: command " + Hex(code) + " needs " + std::to_string(count)
            + " fragments, above " + std::to_string(MAX_FRAGMENTS) + ".");

    Header header;
    header.messageId = messageId;
    header.sequenceLength = static_cast<std::uint32_t>(count);

    const std::span<const Args> records(argsList);
    std::vector<Datagram> datagrams;
    datagrams.reserve(count);
    for (std::size_t i = 0; i < count; i++) {
        const std::size_t first = i * perDatagram;
        header.sequenceNumber = static_cast<std::uint32_t>(i + 1);
        datagrams.push_back(EncodeFragment(*info, code, records.subspan(first, std::min(perDatagram, records.size() - first)), header));
    }
    return datagrams;
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
    if (header.sequenceNumber == 0 || header.sequenceNumber > header.sequenceLength || header.sequenceLength > MAX_FRAGMENTS)
        return Reject(NetError::BadFragment, header.command);

    const CommandInfo *info = FindCommand(header.command);
    if (info == nullptr)
        return Reject(NetError::UnknownCommand, header.command);
    if (info->reserved)
        return Reject(NetError::ReservedCommand, header.command);
    if (!CanReceive(info->direction, role))
        return Reject(NetError::WrongDirection, header.command);
    if (header.sequenceLength != FRAGMENT_COUNT && !info->list)
        return Reject(NetError::BadFragment, header.command);

    const std::size_t recordSize = SchemaSize(info->schema);
    const bool validSize = info->list
        ? recordSize != 0 && header.size % recordSize == 0
        : header.size == recordSize;
    if (!validSize)
        return Reject(NetError::BadPayloadSize, header.command);

    DecodeResult result;
    result.ok = true;
    result.command = static_cast<Command>(header.command);
    result.messageId = header.messageId;
    result.fragmentIndex = header.sequenceNumber;
    result.fragmentCount = header.sequenceLength;
    const std::size_t records = info->list ? header.size / recordSize : 1;
    std::size_t offset = HEADER_SIZE;
    for (std::size_t r = 0; r < records; r++) {
        Args args;
        args.reserve(info->schema.size());
        for (const ArgType type : info->schema) {
            args.push_back(ReadArg(datagram, offset, type));
            offset += ArgSize(type);
        }
        result.args.push_back(std::move(args));
    }
    return result;
}

}
