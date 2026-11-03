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
        const game::Position& position = registre.get<game::Position>(entity);
        const game::Player& player = registre.get<game::Player>(entity);
        net.SendTo(from, Command::PlayerCreation, {{
            ecs::entityValue(entity),
            static_cast<std::int32_t>(position.x),
            static_cast<std::int32_t>(position.y),
            static_cast<std::uint8_t>(player.id - 1)
        }});
        SendInitialState(from);
        return;
    }
    std::uint8_t color = 0;
    while (std::any_of(_players.begin(), _players.end(), [this, color](const auto& player) {
        const game::Player* other = registre.tryGet<game::Player>(player.second);
        return other && other->id - 1 == color;
    }))
        ++color;
    const std::int32_t x = 100;
    const std::int32_t y = 100 + static_cast<std::int32_t>(color) * 80;
    const ecs::Entity entity = game::spawnPlayer(registre, color + 1,
        static_cast<float>(x), static_cast<float>(y));
    _players.emplace(from, entity);
    _synced.insert(ecs::entityValue(entity));
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
