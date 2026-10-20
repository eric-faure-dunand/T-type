/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** broadcast_player_updates.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::BroadcastPlayerUpdates() {
    for (const auto& [endpoint, entity] : _players) {
        if (!registre.alive(entity))
            continue;
        const Position& position = registre.get<Position>(entity);
        const ArgsList args = {{
            ecs::entityValue(entity),
            position.x,
            position.y
        }};
        for (const auto& [client, clientEntity] : _players) {
            if (!net.HasClient(client))
                continue;
            net.SendTo(client, Command::EntityUpdate, args);
        }
    }
}

}
