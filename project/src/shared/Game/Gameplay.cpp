/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Gameplay.cpp
*/

#include "Gameplay.hpp"

namespace r_type::game {

void addGameplaySystems(ecs::SystemManager &systems, std::uint32_t seed, const LevelData &level)
{
    systems.add("player_control", playerControlSystem);
    systems.add("waves", WaveSystem(level, seed));
    systems.add("patterns", patternSystem);
    systems.add("shooting", shootingSystem);
    systems.add("movement", movementSystem);
    systems.add("bounds", boundsSystem);
    systems.add("collision", collisionSystem);
    systems.add("health", healthSystem);
}

}
