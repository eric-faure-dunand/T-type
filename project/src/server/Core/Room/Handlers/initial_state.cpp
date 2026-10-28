/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** initial_state.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::SendInitialState(const Endpoint& client) {
    std::uint32_t count = static_cast<std::uint32_t>(_players.size());
    registre.each<Position, Projectile>(
        [&count](const Position&, const Projectile&) {
            ++count;
        }
    );
    net.SendTo(client, Command::InitialStateBegin, {{count}});
    for (const auto& [endpoint, entity] : _players) {
        if (!registre.alive(entity))
            continue;
        const Position& position = registre.get<Position>(entity);
        net.SendTo(client, Command::EntitySpawn, {{
            ecs::entityValue(entity),
            std::uint8_t{1},
            position.x,
            position.y
        }});
    }
    registre.each<Position, Projectile>(
        [this, &client](ecs::Entity entity, const Position& position, const Projectile&) {
            net.SendTo(client, Command::EntitySpawn, {{
                ecs::entityValue(entity),
                std::uint8_t{2},
                position.x,
                position.y
            }});
        }
    );
    net.SendTo(client, Command::InitialStateEnd, {{count}});
}

void Room::BroadcastPlayerSpawn(const Endpoint& newClient, ecs::Entity entity) {
    if (!registre.alive(entity))
        return;
    const Position& position = registre.get<Position>(entity);
    for (const auto& [endpoint, existingEntity] : _players) {
        if (endpoint == newClient)
            continue;
        net.SendTo(endpoint, Command::EntitySpawn, {{
            ecs::entityValue(entity),
            std::uint8_t{1},
            position.x,
            position.y
        }});
    }
}

}
