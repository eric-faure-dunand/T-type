/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** connection.cpp
*/

#include "ClientCore.hpp"

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

void ClientCore::RegisterHandlers() {
    _net.On(Command::ConnectAccept, [this](const Endpoint&, const ArgsList&) {OnConnectAccept();});
    _net.On(Command::Disconnect, [this](const Endpoint&, const ArgsList&) {OnDisconnect();});
    _net.On(Command::Error, [this](const Endpoint&, const ArgsList& args) {OnError(args);});
    _net.On(Command::RoomList, [this](const Endpoint&, const ArgsList& args) {OnRoomList(args);});
    _net.On(Command::RoomJoined, [this](const Endpoint&, const ArgsList& args) {OnRoomJoined(args);});
    _net.On(Command::PlayerCreation, [this](const Endpoint&, const ArgsList& args) {OnPlayerCreation(args);});
    _net.On(Command::InitialStateBegin, [this](const Endpoint&, const ArgsList& args) {OnInitialStateBegin(args);});
    _net.On(Command::InitialStateEnd, [this](const Endpoint&, const ArgsList& args) {OnInitialStateEnd(args);});
    _net.On(Command::EntitySpawn, [this](const Endpoint&, const ArgsList& args) {OnEntitySpawn(args);});
    _net.On(Command::EntityUpdate, [this](const Endpoint&, const ArgsList& args) {OnEntityUpdate(args);});
    _net.On(Command::EntityDestroy, [this](const Endpoint&, const ArgsList& args) {OnEntityDestroy(args);});
}

void ClientCore::OnConnectAccept() {
    if (_scene == Scene::Connecting) {
        _rooms.clear();
        _selected = 0;
        SetScene(Scene::Lobby);
        Send(Command::ListRooms, {{}});
    } else if (_scene == Scene::Joining && _roomPort != 0) {
        SetScene(Scene::Game);
    } else if (_scene == Scene::Leaving) {
        SetScene(Scene::Lobby);
        Send(Command::ListRooms, {{}});
    }
}

void ClientCore::OnDisconnect() {
    if (_scene == Scene::Game || (_scene == Scene::Joining && _roomPort != 0)) {
        ShowMessage("You were removed from the room");
        ReturnToLobby();
    } else if (_scene != Scene::Menu) {
        GoOffline("Disconnected by the server");
    }
}

void ClientCore::OnError(const ArgsList& args) {
    if (args.size() != 1 || args[0].size() != 2)
        return;
    const NetError error = static_cast<NetError>(std::get<std::uint8_t>(args[0][0]));
    switch (_scene) {
        case Scene::Connecting:
            GoOffline(Describe(error));
            break;
        case Scene::Joining:
            if (_roomPort == 0 && (error == NetError::RoomNotFound || error == NetError::AlreadyInRoom
                || error == NetError::TooManyRooms)) {
                ShowMessage(Describe(error));
                SetScene(Scene::Lobby);
            } else if (_roomPort != 0 && (error == NetError::Full || error == NetError::NotConnected)) {
                ShowMessage(error == NetError::Full ? "This room is full" : "Could not enter the room");
                ReturnToLobby();
            }
            break;
        case Scene::Lobby:
            if (error == NetError::NotConnected)
                GoOffline("Connection to the server lost");
            break;
        case Scene::Game:
            if (error == NetError::NotConnected) {
                ShowMessage("The room closed");
                ReturnToLobby();
            }
            break;
        case Scene::Leaving:
            if (error == NetError::Full || error == NetError::NotConnected)
                GoOffline("Connection to the server lost");
            break;
        case Scene::Menu:
            break;
    }
}

}
