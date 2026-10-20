/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** player_connection.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::HandlePlayerConnection(const Endpoint& from) {
    if (!net.HasClient(from))
        return;
    const auto existing = _players.find(from);
    if (existing != _players.end()) {
        const ecs::Entity entity = existing->second;
        if (!registre.alive(entity))
            return;
        const Position& position = registre.get<Position>(entity);
        const Player& player = registre.get<Player>(entity);
        net.SendTo(from, Command::PlayerCreation, {{
            ecs::entityValue(entity),
            position.x,
            position.y,
            player.color
        }});
        SendInitialState(from);
        return;
    }
    const ecs::Entity entity = registre.spawn();
    const std::uint8_t color = static_cast<std::uint8_t>(_players.size() % 4);
    const std::int32_t x = 100;
    const std::int32_t y = 100 + static_cast<std::int32_t>(_players.size()) * 80;
    registre.add<Position>(entity, Position{x, y});
    registre.add<Player>(entity, Player{color});
    _players.emplace(from, entity);
    net.SendTo(from, Command::PlayerCreation, {{
        ecs::entityValue(entity),
        x,
        y,
        color
    }});
    SendInitialState(from);
    BroadcastPlayerSpawn(from, entity);
}

}
