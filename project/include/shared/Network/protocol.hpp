#ifndef PROTOCOL_HPP
    #define PROTOCOL_HPP

    #include <cstddef>
    #include <cstdint>
    #include <map>
    #include <vector>

namespace r_type {

constexpr std::uint16_t MAGIC_NUMBER = 0x5254;
constexpr std::size_t HEADER_SIZE = 13;
constexpr std::size_t MAX_DATAGRAM_SIZE = 1200;
constexpr std::size_t MAX_PAYLOAD_SIZE = MAX_DATAGRAM_SIZE - HEADER_SIZE;

constexpr std::uint32_t FRAGMENT_INDEX = 1;
constexpr std::uint32_t FRAGMENT_COUNT = 1;

enum class Command : std::uint8_t {
    ConnectRequest = 0x01,
    ConnectAccept = 0x02,
    Disconnect = 0x03,
    CreateRoom = 0x04,
    JoinRoom = 0x05,
    RoomJoined = 0x06,
    ListRooms = 0x07,
    RoomList = 0x08,
    DisconnectRoom = 0x09,
    PlayerInput = 0x10,
    InitialStateBegin = 0x20,
    EntityUpdate = 0x21,
    EntitySpawn = 0x22,
    EntityDestroy = 0x23,
    InitialStateEnd = 0x24,
    Ack = 0x30,
    Error = 0x84,
};

enum class NetError : std::uint8_t {
    DatagramTooShort = 0x01,
    DatagramTooLong = 0x02,
    BadMagic = 0x03,
    SizeMismatch = 0x04,
    FragmentationUnsupported = 0x05,
    UnknownCommand = 0x06,
    ReservedCommand = 0x07,
    WrongDirection = 0x08,
    BadPayloadSize = 0x09,
    NotConnected = 0x0A,
    Full = 0x0B,
    RoomNotFound = 0x0D,
    CommandNotHandled = 0x0E,
    AlreadyInRoom = 0x0F,
    TooManyRooms = 0x10,
};

enum class ArgType : std::uint8_t {
    U8,
    U32,
    I32,
};

enum class Direction : std::uint8_t {
    ClientToServer,
    ServerToClient,
    Both,
};

enum class Role : std::uint8_t {
    Server,
    Client,
};

struct CommandInfo {
    Direction direction;
    std::vector<ArgType> schema;
    bool list = false;
    bool reserved = false;
};

const CommandInfo *FindCommand(std::uint8_t code);
std::size_t ArgSize(ArgType type);
std::size_t SchemaSize(const std::vector<ArgType>& schema);
bool CanSend(Direction direction, Role role);
bool CanReceive(Direction direction, Role role);

}

#endif
