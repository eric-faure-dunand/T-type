#include "protocol.hpp"

namespace r_type {

static const std::map<Command, CommandInfo> COMMANDS = {
    // (empty)
    {Command::ConnectRequest, {Direction::ClientToServer, {}}},
    // (empty)
    {Command::ConnectAccept, {Direction::ServerToClient, {}}},
    // (empty)
    {Command::Disconnect, {Direction::Both, {}}},
    // (empty)
    {Command::CreateRoom, {Direction::ClientToServer, {}}},
    // room_id
    {Command::JoinRoom, {Direction::ClientToServer, {ArgType::U32}}},
    // port
    {Command::RoomJoined, {Direction::ServerToClient, {ArgType::U32}}},
    // (empty)
    {Command::ListRooms, {Direction::ClientToServer, {}}},
    // list of: room_id, free_slots
    {Command::RoomList, {Direction::ServerToClient, {ArgType::U32, ArgType::U8}, true}},
    // entity_id
    {Command::DisconnectRoom, {Direction::ClientToServer, {ArgType::U32}}},
    // entity_id, input_flags
    {Command::PlayerInput, {Direction::ClientToServer, {ArgType::U32, ArgType::U8}}},
    // entity_count
    {Command::InitialStateBegin, {Direction::ServerToClient, {ArgType::U32}}},
    // entity_id, x, y
    {Command::EntityUpdate, {Direction::ServerToClient, {ArgType::U32, ArgType::I32, ArgType::I32}}},
    // entity_id, entity_kind, x, y
    {Command::EntitySpawn, {Direction::ServerToClient, {ArgType::U32, ArgType::U8, ArgType::I32, ArgType::I32}}},
    // entity_id
    {Command::EntityDestroy, {Direction::ServerToClient, {ArgType::U32}}},
    // entity_count
    {Command::InitialStateEnd, {Direction::ServerToClient, {ArgType::U32}}},
    // reserved for reliable delivery, rejected in Part 1
    {Command::Ack, {Direction::Both, {}, false, true}},
    // error_code, rejected_command
    {Command::Error, {Direction::Both, {ArgType::U8, ArgType::U8}}},
};

const CommandInfo *FindCommand(std::uint8_t code) {
    const auto it = COMMANDS.find(static_cast<Command>(code));
    if (it == COMMANDS.end())
        return nullptr;
    return &it->second;
}

std::size_t ArgSize(ArgType type) {
    switch (type) {
        case ArgType::U8:
            return 1;
        case ArgType::U32:
        case ArgType::I32:
            return 4;
    }
    return 0;
}

std::size_t SchemaSize(const std::vector<ArgType>& schema) {
    std::size_t size = 0;
    for (const ArgType type : schema)
        size += ArgSize(type);
    return size;
}

bool CanSend(Direction direction, Role role) {
    if (direction == Direction::Both)
        return true;
    if (role == Role::Server)
        return direction == Direction::ServerToClient;
    return direction == Direction::ClientToServer;
}

bool CanReceive(Direction direction, Role role) {
    return CanSend(direction, role == Role::Server ? Role::Client : Role::Server);
}

}
