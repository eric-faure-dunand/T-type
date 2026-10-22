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
    const std::uint32_t entityId = ecs::entityValue(entity);
    for (const auto& [endpoint, otherEntity] : _players) {
        if (endpoint == from)
            continue;
        if (!net.HasClient(endpoint))
            continue;
        net.SendTo(endpoint, Command::EntityDestroy, {{entityId}});
    }
    if (registre.alive(entity))
        registre.kill(entity);
    _players.erase(it);
    net.RemoveClient(from);
}

}
