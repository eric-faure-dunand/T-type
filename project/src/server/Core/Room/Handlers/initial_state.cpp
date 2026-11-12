/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** initial_state.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::SendInitialState(const Endpoint& client) {
    std::uint32_t count = 0;
    registre.each<game::Kind, game::Position>(
        [&count](const game::Kind&, const game::Position&) {
            ++count;
        }
    );
    net.SendTo(client, Command::InitialStateBegin, {{count}});
    registre.each<game::Kind, game::Position>(
        [this, &client](ecs::Entity entity, const game::Kind&, const game::Position&) {
            net.SendTo(client, Command::EntitySpawn, SpawnArgs(entity));
        }
    );
    net.SendTo(client, Command::InitialStateEnd, {{count}});
    for (const auto& [id, percent] : _bossHealth)
        net.SendTo(client, Command::BossHealth, {{id, percent}});
}

void Room::BroadcastPlayerSpawn(const Endpoint& newClient, ecs::Entity entity) {
    if (!registre.alive(entity))
        return;
    const ArgsList args = SpawnArgs(entity);
    for (const auto& [endpoint, existingEntity] : _players) {
        if (endpoint == newClient)
            continue;
        net.SendTo(endpoint, Command::EntitySpawn, args);
    }
}

}
