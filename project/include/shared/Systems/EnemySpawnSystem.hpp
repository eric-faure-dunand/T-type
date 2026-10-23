/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** EnemySpawnSystem.hpp
*/

#ifndef SYSTEMS_ENEMY_SPAWN_HPP
    #define SYSTEMS_ENEMY_SPAWN_HPP

    #include <cstdint>
    #include <random>

    #include "Registry.hpp"

namespace r_type::game {

class EnemySpawnSystem {
public:
    explicit EnemySpawnSystem(std::uint32_t seed);

    void operator()(ecs::Registry &r, float dt);

private:
    std::mt19937 rng_;
    float timer_ = 0.f;
};

}

#endif
