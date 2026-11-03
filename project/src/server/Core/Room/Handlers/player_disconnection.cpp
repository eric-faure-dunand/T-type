/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** player_disconnection.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::HandlePlayerDisconnection(const Endpoint& from) {
    const auto it = _players.find(from);
    if (it == _players.end()) {
        net.RemoveClient(from);
        return;
    }
    const ecs::Entity entity = it->second;
    if (registre.alive(entity))
        registre.kill(entity);
    _players.erase(it);
    net.RemoveClient(from);
}

}
