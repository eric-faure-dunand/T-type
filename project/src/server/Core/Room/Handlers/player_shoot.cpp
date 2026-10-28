/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** player_shoot.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::HandlePlayerShoot(ecs::Entity playerEntity) {
    if (!registre.alive(playerEntity))
        return;
    const std::uint32_t playerId = ecs::entityValue(playerEntity);
    const auto now = std::chrono::steady_clock::now();
    const auto lastShot = _lastShotTime.find(playerId);
    if (lastShot != _lastShotTime.end()) {
        const auto elapsed = now - lastShot->second;
        if (elapsed < std::chrono::milliseconds(200))
            return;
    }
    _lastShotTime[playerId] = now;
    const Position& playerPosition = registre.get<Position>(playerEntity);
    const ecs::Entity projectile = registre.spawn();
    registre.add<Position>(projectile, Position{
        playerPosition.x + 40,
        playerPosition.y
    });
    registre.add<Projectile>(projectile, Projectile{
        ecs::entityValue(playerEntity),
        10
    });
    const Position& position = registre.get<Position>(projectile);
    for (const auto& [client, entity] : _players) {
        if (!net.HasClient(client))
            continue;
        net.SendTo(client, Command::EntitySpawn, {{
            ecs::entityValue(projectile),
            std::uint8_t{2},
            position.x,
            position.y
        }});
    }
}

}
