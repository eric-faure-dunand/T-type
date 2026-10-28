/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** broadcast_projectile_updates.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::BroadcastProjectileUpdates() {
    registre.each<Position, Projectile>(
        [this](ecs::Entity entity, const Position& position, const Projectile&) {
            const ArgsList args = {{
                ecs::entityValue(entity),
                position.x,
                position.y
            }};
            for (const auto& [client, playerEntity] : _players) {
                if (!net.HasClient(client))
                    continue;
                net.SendTo(client, Command::EntityUpdate, args);
            }
        }
    );
}

}
