#include "protocol.hpp"

namespace r_type {

static const std::map<Command, CommandInfo> COMMANDS = {
    // (empty)
    {Command::ConnectRequest, {Direction::ClientToServer, {}, false, false, true}},
    // (empty)
    {Command::ConnectAccept, {Direction::ServerToClient, {}, false, false, true}},
    // (empty)
    {Command::Disconnect, {Direction::Both, {}, false, false, true}},
    // (empty)
    {Command::CreateRoom, {Direction::ClientToServer, {}, false, false, true}},
    // room_id
    {Command::JoinRoom, {Direction::ClientToServer, {ArgType::U32}, false, false, true}},
    // port
    {Command::RoomJoined, {Direction::ServerToClient, {ArgType::U32}, false, false, true}},
    // (empty)
    {Command::ListRooms, {Direction::ClientToServer, {}, false, false, true}},
    // list of: room_id, free_slots
    {Command::RoomList, {Direction::ServerToClient, {ArgType::U32, ArgType::U8}, true, false, true}},
    // entity_id
    {Command::DisconnectRoom, {Direction::ClientToServer, {ArgType::U32}, false, false, true}},
    // entity_id, input_flags, input_state
    {Command::PlayerInput, {Direction::ClientToServer, {ArgType::U32, ArgType::U8, ArgType::U8}}},
    // entity_id, character
    {Command::ChooseCharacter, {Direction::ClientToServer, {ArgType::U32, ArgType::U8}, false, false, true}},
    // entity_count
    {Command::InitialStateBegin, {Direction::ServerToClient, {ArgType::U32}, false, false, true}},
    // list of: entity_id, x, y
    {Command::EntityUpdate, {Direction::ServerToClient, {ArgType::U32, ArgType::I32, ArgType::I32}, true}},
    // list of: entity_id, entity_kind, x, y, color
    {Command::EntitySpawn, {Direction::ServerToClient, {ArgType::U32, ArgType::U8, ArgType::I32, ArgType::I32, ArgType::U8}, true, false, true}},
    // list of: entity_id
    {Command::EntityDestroy, {Direction::ServerToClient, {ArgType::U32}, true, false, true}},
    // entity_count
    {Command::InitialStateEnd, {Direction::ServerToClient, {ArgType::U32}, false, false, true}},
    // entity_id, x, y, color
    {Command::PlayerCreation, {Direction::ServerToClient, {ArgType::U32, ArgType::I32, ArgType::I32, ArgType::U8}, false, false, true}},
    {Command::Ack, {Direction::Both, {ArgType::U32}, true}},
    {Command::Resend, {Direction::Both, {ArgType::U32, ArgType::U32}, true}},
    // Internal secure handshake confirmation
    {Command::HandshakeReady, {Direction::ServerToClient, {}, false, false, false}},
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
