/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** WaveSystem.cpp
*/

#include "WaveSystem.hpp"

namespace r_type::game {

WaveSystem::WaveSystem(const LevelData &level, std::uint32_t seed) : boss_(level.boss), rng_(seed)
{
    for (const WaveData &wave : level.waves)
        for (int i = 0; i < wave.count; i++)
            spawns_.push_back({wave.time + static_cast<float>(i) * wave.spacing, wave, i});
    std::stable_sort(spawns_.begin(), spawns_.end(), [](const Spawn &a, const Spawn &b) {
        return a.time < b.time;
    });
}

void WaveSystem::operator()(ecs::Registry &r, float dt)
{
    if (spawns_.empty() && !boss_.enabled)
        return;
    time_ += dt;
    while (next_ < spawns_.size() && spawns_[next_].time <= time_)
        spawn(r, spawns_[next_++]);
    if (boss_.enabled && !bossEntity_ && time_ >= boss_.time)
        bossEntity_ = spawnBoss(r, boss_.type, boss_.health);
    if (!restartAt_ && finished(r))
        restartAt_ = time_ + kLevelRestartDelay;
    if (restartAt_ && time_ >= *restartAt_)
        restart();
}

bool WaveSystem::finished(const ecs::Registry &r) const
{
    if (next_ < spawns_.size())
        return false;
    return !boss_.enabled || (bossEntity_ && !r.alive(*bossEntity_));
}

void WaveSystem::restart()
{
    time_ = 0.f;
    next_ = 0;
    bossEntity_.reset();
    restartAt_.reset();
}

void WaveSystem::spawn(ecs::Registry &r, const Spawn &spawn)
{
    const WaveData &wave = spawn.wave;
    const float top = kEnemySize;
    const float bottom = kWorldHeight - 2.f * kEnemySize;
    float x = kWorldWidth;
    float y = std::uniform_real_distribution<float>(top, bottom)(rng_);
    if (wave.pattern == PatternType::Wall) {
        const float step = wave.count > 1 ? (bottom - top) / static_cast<float>(wave.count - 1) : 0.f;
        y = top + static_cast<float>(spawn.index) * step;
    } else if (wave.pattern == PatternType::Formation) {
        const int rows = std::min(wave.count, kFormationRows);
        const float step = rows > 1 ? (bottom - top) / static_cast<float>(rows - 1) : 0.f;
        y = top + static_cast<float>(spawn.index % rows) * step;
        x += static_cast<float>(spawn.index / rows) * kFormationSpacing;
    }
    const float fireRate = kEnemyStats[static_cast<std::size_t>(wave.enemy)].fireRate;
    const float firstShot = fireRate > 0.f ? std::uniform_real_distribution<float>(0.5f, 1.f / fireRate)(rng_) : 0.f;
    spawnEnemy(r, wave.enemy, wave.pattern, x, y, firstShot);
}

}
