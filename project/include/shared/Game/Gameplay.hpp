/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Gameplay.hpp
*/

#ifndef GAME_GAMEPLAY_HPP
    #define GAME_GAMEPLAY_HPP

    #include <cstdint>

    #include "shared/ECS/SystemManager.hpp"
    #include "shared/Systems/BoundsSystem.hpp"
    #include "shared/Systems/CollisionSystem.hpp"
    #include "shared/Systems/EnemySpawnSystem.hpp"
    #include "shared/Systems/HealthSystem.hpp"
    #include "shared/Systems/MovementSystem.hpp"
    #include "shared/Systems/PlayerControlSystem.hpp"
    #include "shared/Systems/ShootingSystem.hpp"
    #include "Spawner.hpp"

namespace r_type::game {

inline void addGameplaySystems(ecs::SystemManager &systems, std::uint32_t seed)
{
    systems.add("player_control", playerControlSystem);
    systems.add("shooting", shootingSystem);
    systems.add("movement", movementSystem);
    systems.add("bounds", boundsSystem);
    systems.add("collision", collisionSystem);
    systems.add("health", healthSystem);
    systems.add("enemy_spawn", EnemySpawnSystem(seed));
}

}

#endif
