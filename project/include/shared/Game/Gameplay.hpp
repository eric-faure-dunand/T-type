/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Gameplay.hpp
*/

#ifndef GAME_GAMEPLAY_HPP
    #define GAME_GAMEPLAY_HPP

    #include <cstdint>

    #include "SystemManager.hpp"
    #include "BoundsSystem.hpp"
    #include "CollisionSystem.hpp"
    #include "EnemySpawnSystem.hpp"
    #include "HealthSystem.hpp"
    #include "MovementSystem.hpp"
    #include "PlayerControlSystem.hpp"
    #include "ShootingSystem.hpp"

namespace r_type::game {

void addGameplaySystems(ecs::SystemManager &systems, std::uint32_t seed);

}

#endif
