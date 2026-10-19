/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** handlers.cpp
*/

#include "ClientNetwork.hpp"

namespace r_type::client {

static std::string Describe(NetError error) {
    switch (error) {
        case NetError::Full:
            return "Server is full";
        case NetError::RoomNotFound:
            return "This room does not exist anymore";
        case NetError::AlreadyInRoom:
            return "You are already in a room";
        case NetError::TooManyRooms:
            return "Too many rooms, try again later";
        case NetError::NotConnected:
            return "Not connected to the server";
        default:
            return "Network error";
    }
}

static bool HasRecord(const ArgsList& args, std::size_t size) {
    return args.size() == 1 && args[0].size() == size;
}

void ClientNetwork::RegisterHandlers() {
    _socket.On(Command::ConnectAccept, [this](const Endpoint&, const ArgsList&) {OnConnectAccept();});
    _socket.On(Command::Disconnect, [this](const Endpoint&, const ArgsList&) {OnDisconnect();});
    _socket.On(Command::RoomList, [this](const Endpoint&, const ArgsList& args) {OnRoomList(args);});
    _socket.On(Command::RoomJoined, [this](const Endpoint&, const ArgsList& args) {OnRoomJoined(args);});
    _socket.On(Command::Error, [this](const Endpoint&, const ArgsList& args) {OnError(args);});
    _socket.On(Command::PlayerCreation, [this](const Endpoint&, const ArgsList& args) {OnPlayerCreation(args);});
    _socket.On(Command::InitialStateBegin, [this](const Endpoint&, const ArgsList& args) {OnInitialStateBegin(args);});
    _socket.On(Command::InitialStateEnd, [this](const Endpoint&, const ArgsList& args) {OnInitialStateEnd(args);});
    _socket.On(Command::EntitySpawn, [this](const Endpoint&, const ArgsList& args) {OnEntitySpawn(args);});
    _socket.On(Command::EntityUpdate, [this](const Endpoint&, const ArgsList& args) {OnEntityUpdate(args);});
    _socket.On(Command::EntityDestroy, [this](const Endpoint&, const ArgsList& args) {OnEntityDestroy(args);});
}

void ClientNetwork::OnConnectAccept() {
    if (_stage == Stage::ConnectingServer) {
        SetStage(Stage::Lobby);
        Emit(event::Connected{});
        Send(Command::ListRooms, {{}});
    } else if (_stage == Stage::JoiningRoom) {
        SetStage(Stage::InRoom);
        Emit(event::RoomEntered{});
    } else if (_stage == Stage::ReturningToLobby) {
        SetStage(Stage::Lobby);
        Emit(event::BackToLobby{});
        Send(Command::ListRooms, {{}});
    }
}

void ClientNetwork::OnDisconnect() {
    if (_stage == Stage::InRoom || _stage == Stage::JoiningRoom) {
        Emit(event::Notice{"You were removed from the room"});
        ReturnToLobby();
    } else if (_stage != Stage::Offline) {
        GoOffline("Disconnected by the server", false);
    }
}

void ClientNetwork::OnRoomList(const ArgsList& args) {
    if (_stage != Stage::Lobby)
        return;
    std::vector<RoomInfo> rooms;
    for (const Args& record : args) {
        if (record.size() != 2)
            continue;
        rooms.push_back({std::get<std::uint32_t>(record[0]), std::get<std::uint8_t>(record[1])});
    }
    Emit(event::Rooms{std::move(rooms)});
}

void ClientNetwork::OnRoomJoined(const ArgsList& args) {
    if (_stage != Stage::Lobby || !HasRecord(args, 1))
        return;
    const std::uint32_t port = std::get<std::uint32_t>(args[0][0]);
    if (port == 0 || port > 0xFFFF)
        return;
    try {
        _roomPort = static_cast<std::uint16_t>(port);
        _socket.Connect(_ip, _roomPort);
        SetStage(Stage::JoiningRoom);
    } catch (const IError&) {
        Emit(event::Notice{"Could not enter the room"});
    }
}

void ClientNetwork::OnError(const ArgsList& args) {
    if (!HasRecord(args, 2))
        return;
    const NetError error = static_cast<NetError>(std::get<std::uint8_t>(args[0][0]));
    switch (_stage) {
        case Stage::ConnectingServer:
            GoOffline(Describe(error), true);
            break;
        case Stage::JoiningRoom:
            if (error == NetError::Full || error == NetError::NotConnected) {
                Emit(event::Notice{error == NetError::Full ? "This room is full" : "Could not enter the room"});
                ReturnToLobby();
            }
            break;
        case Stage::Lobby:
            if (error == NetError::NotConnected)
                GoOffline("Connection to the server lost", false);
            else if (error == NetError::RoomNotFound || error == NetError::AlreadyInRoom
                || error == NetError::TooManyRooms)
                Emit(event::Notice{Describe(error)});
            break;
        case Stage::InRoom:
            if (error == NetError::NotConnected) {
                Emit(event::Notice{"The room closed"});
                ReturnToLobby();
            }
            break;
        case Stage::ReturningToLobby:
            if (error == NetError::Full || error == NetError::NotConnected)
                GoOffline("Connection to the server lost", false);
            break;
        case Stage::Offline:
            break;
    }
}

void ClientNetwork::OnPlayerCreation(const ArgsList& args) {
    if ((_stage != Stage::InRoom && _stage != Stage::JoiningRoom) || !HasRecord(args, 4))
        return;
    const event::PlayerCreated player{std::get<std::uint32_t>(args[0][0]), std::get<std::int32_t>(args[0][1]),
        std::get<std::int32_t>(args[0][2]), std::get<std::uint8_t>(args[0][3])};
    _player = player.id;
    Emit(player);
}

void ClientNetwork::OnInitialStateBegin(const ArgsList& args) {
    if (_stage != Stage::InRoom || !HasRecord(args, 1))
        return;
    const std::uint32_t count = std::get<std::uint32_t>(args[0][0]);
    if (count > MAX_TRANSFER_ENTITIES)
        return;
    _transfer = Transfer{count, {}};
}

void ClientNetwork::OnInitialStateEnd(const ArgsList& args) {
    if (!_transfer || !HasRecord(args, 1))
        return;
    const std::uint32_t count = std::get<std::uint32_t>(args[0][0]);
    if (count == _transfer->expected && _transfer->entities.size() == count) {
        std::vector<EntityData> entities;
        for (const auto& [id, entity] : _transfer->entities)
            entities.push_back(entity);
        Emit(event::Snapshot{std::move(entities)});
    } else {
        try {
            _socket.Connect(_ip, _roomPort);
        } catch (const IError&) {
            Emit(event::Notice{"Could not load the game state"});
        }
    }
    _transfer.reset();
}

void ClientNetwork::OnEntitySpawn(const ArgsList& args) {
    if (_stage != Stage::InRoom || !HasRecord(args, 4))
        return;
    const EntityData entity{std::get<std::uint32_t>(args[0][0]), std::get<std::uint8_t>(args[0][1]),
        std::get<std::int32_t>(args[0][2]), std::get<std::int32_t>(args[0][3])};
    if (_destroyed.contains(entity.id))
        return;
    if (_transfer) {
        if (_transfer->entities.size() < _transfer->expected || _transfer->entities.contains(entity.id))
            _transfer->entities[entity.id] = entity;
        return;
    }
    Emit(event::Spawn{entity});
}

void ClientNetwork::OnEntityUpdate(const ArgsList& args) {
    if (_stage != Stage::InRoom || !HasRecord(args, 3))
        return;
    const std::uint32_t id = std::get<std::uint32_t>(args[0][0]);
    if (_destroyed.contains(id))
        return;
    Emit(event::Update{id, std::get<std::int32_t>(args[0][1]), std::get<std::int32_t>(args[0][2])});
}

void ClientNetwork::OnEntityDestroy(const ArgsList& args) {
    if (_stage != Stage::InRoom || !HasRecord(args, 1))
        return;
    const std::uint32_t id = std::get<std::uint32_t>(args[0][0]);
    _destroyed.insert(id);
    if (_transfer)
        _transfer->entities.erase(id);
    Emit(event::Destroy{id});
}

}
