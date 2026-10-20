/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** rooms.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::OnRoomList(const ArgsList& args) {
    if (_scene != Scene::Lobby)
        return;
    _rooms.clear();
    for (const Args& record : args)
        if (record.size() == 2)
            _rooms.push_back({std::get<std::uint32_t>(record[0]), std::get<std::uint8_t>(record[1])});
    if (_selected >= _rooms.size())
        _selected = _rooms.empty() ? 0 : _rooms.size() - 1;
}

void ClientCore::OnRoomJoined(const ArgsList& args) {
    if (_scene != Scene::Joining || _roomPort != 0 || args.size() != 1 || args[0].size() != 1)
        return;
    const std::uint32_t port = std::get<std::uint32_t>(args[0][0]);
    if (port == 0 || port > 0xFFFF)
        return;
    try {
        _roomPort = static_cast<std::uint16_t>(port);
        _net.Connect(_ip, _roomPort);
        SetScene(Scene::Joining);
    } catch (const IError&) {
        _roomPort = 0;
        ShowMessage("Could not enter the room");
        SetScene(Scene::Lobby);
    }
}

}
