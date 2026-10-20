/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** connection.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

bool ClientCore::Send(Command command, const ArgsList& args) {
    try {
        _net.Send(command, args);
    } catch (const IError&) {
        return false;
    }
    return true;
}

void ClientCore::ConnectServer() {
    try {
        _net.Close();
        _net.Connect(_ip, DEFAULT_PORT);
        SetScene(Scene::Connecting);
    } catch (const IError&) {
        _net.Close();
        ShowMessage("Invalid IP address: " + _ip);
    }
}

void ClientCore::ReturnToLobby() {
    ResetRoom();
    try {
        _net.Connect(_ip, _net.ServerPort());
        SetScene(Scene::Leaving);
    } catch (const IError&) {
        GoOffline("Connection to the server lost");
    }
}

void ClientCore::GoOffline(const std::string& reason) {
    ResetRoom();
    _net.Close();
    _rooms.clear();
    SetScene(Scene::Menu);
    if (!reason.empty())
        ShowMessage(reason);
}

void ClientCore::SayGoodbye() {
    if (_scene == Scene::Menu || !_net.IsOpen())
        return;
    if (_scene == Scene::Game)
        Send(Command::DisconnectRoom, {{_world.Player().value_or(0)}});
    try {
        asio::error_code ec;
        const asio::ip::address address = asio::ip::make_address(_ip, ec);
        if (!ec)
            _net.SendTo(Endpoint(address, _net.ServerPort()), Command::Disconnect, {{}});
    } catch (const IError&) {
        return;
    }
}

void ClientCore::ResetRoom() {
    _world.Clear();
    _destroyed.clear();
    _transfer.reset();
    _roomPort = 0;
    _held.clear();
    _releases.clear();
}

void ClientCore::SetHeld(const std::set<Action>& held) {
    for (const Action action : held) {
        if (_held.contains(action))
            continue;
        _releases.erase(action);
        SendInput(action, InputState::Pressed);
    }
    for (const Action action : _held) {
        if (held.contains(action))
            continue;
        _releases[action] = RELEASE_RESENDS;
        SendInput(action, InputState::Released);
    }
    _held = held;
}

void ClientCore::SendInput(Action action, InputState state) {
    const std::optional<std::uint32_t> player = _world.Player();
    if (!player)
        return;
    Send(Command::PlayerInput, {{*player, static_cast<std::uint8_t>(action), static_cast<std::uint8_t>(state)}});
}

void ClientCore::ResendInputs(float dt) {
    _inputTimer += dt;
    if (_inputTimer < INPUT_RESEND_DELAY)
        return;
    _inputTimer = 0.f;
    for (const Action action : _held)
        SendInput(action, InputState::Pressed);
    for (auto it = _releases.begin(); it != _releases.end();) {
        SendInput(it->first, InputState::Released);
        it = --it->second > 0 ? std::next(it) : _releases.erase(it);
    }
}

}
