/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** destroy_out_of_bounds_projectiles.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::DestroyOutOfBoundsProjectiles() {
    std::vector<ecs::Entity> toDestroy;

    registre.each<Position, Projectile>(
        [&toDestroy](ecs::Entity entity, const Position& position, const Projectile&) {
            if (position.x >= 1280)
                toDestroy.push_back(entity);
        }
    );
    for (const ecs::Entity entity : toDestroy) {
        const std::uint32_t entityId = ecs::entityValue(entity);
        for (const auto& [client, playerEntity] : _players) {
            if (net.HasClient(client))
                net.SendTo(client, Command::EntityDestroy, {{entityId}});
        }
        registre.kill(entity);
    }
}

}
