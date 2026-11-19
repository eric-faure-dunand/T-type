/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** WaveSystem.hpp
*/

#ifndef SYSTEMS_WAVE_HPP
    #define SYSTEMS_WAVE_HPP

    #include <algorithm>
    #include <cstddef>
    #include <cstdint>
    #include <optional>
    #include <random>
    #include <vector>

    #include "Registry.hpp"
    #include "GameConfig.hpp"
    #include "Level.hpp"
    #include "Spawner.hpp"

namespace r_type::game {

class WaveSystem {
public:
    WaveSystem(const LevelData &level, std::uint32_t seed);

    void operator()(ecs::Registry &r, float dt);

private:
    struct Spawn {
        float time;
        WaveData wave;
        int index;
    };

    void spawn(ecs::Registry &r, const Spawn &spawn);
    bool finished(const ecs::Registry &r) const;
    void restart();

    std::vector<Spawn> spawns_;
    BossData boss_;
    std::optional<ecs::Entity> bossEntity_;
    std::size_t next_ = 0;
    float time_ = 0.f;
    std::optional<float> restartAt_;
    std::mt19937 rng_;
};

}

#endif
