/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** EnemySpawnSystem.cpp
*/

#include "EnemySpawnSystem.hpp"

namespace r_type::game {

EnemySpawnSystem::EnemySpawnSystem(std::uint32_t seed) : rng_(seed) {}

void EnemySpawnSystem::operator()(ecs::Registry &r, float dt)
{
    timer_ -= dt;
    if (timer_ > 0.f)
        return;
    timer_ += kEnemySpawnInterval;
    std::uniform_real_distribution<float> height(0.f, kWorldHeight - kEnemySize);
    std::uniform_real_distribution<float> firstShot(0.2f, 1.f / kEnemyFireRate);
    spawnEnemy(r, kWorldWidth, height(rng_), firstShot(rng_));
}

}
