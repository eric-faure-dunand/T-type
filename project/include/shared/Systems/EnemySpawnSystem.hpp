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

    #include "shared/ECS/Registry.hpp"
    #include "shared/Game/GameConfig.hpp"
    #include "shared/Game/Spawner.hpp"

namespace r_type::game {

class EnemySpawnSystem {
public:
    explicit EnemySpawnSystem(std::uint32_t seed) : rng_(seed) {}

    void operator()(ecs::Registry &r, float dt)
    {
        timer_ -= dt;
        if (timer_ > 0.f)
            return;
        timer_ += kEnemySpawnInterval;
        std::uniform_real_distribution<float> height(0.f, kWorldHeight - kEnemySize);
        std::uniform_real_distribution<float> firstShot(0.2f, 1.f / kEnemyFireRate);
        spawnEnemy(r, kWorldWidth, height(rng_), firstShot(rng_));
    }

private:
    std::mt19937 rng_;
    float timer_ = 0.f;
};

}

#endif
