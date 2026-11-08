/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** LevelData.hpp
*/

#ifndef LEVEL_LEVEL_DATA_HPP
    #define LEVEL_LEVEL_DATA_HPP

    #include <cstddef>
    #include <cstdint>
    #include <optional>
    #include <string>
    #include <vector>

namespace r_type::level {

constexpr std::size_t kMaxWaves = 500;
constexpr int kMaxCountPerWave = 100;
constexpr int kMaxBossHealth = 100000;
constexpr double kMaxTimeSeconds = 3600.0;
constexpr std::size_t kMaxFileBytes = 1024 * 1024;

enum class EnemyType : std::uint8_t {
    Ballon,
    Supporter,
    Defenseur,
    Scorpion,
    Harissa,
    Simit,
    Cay,
    Chat,
    Grognard,
    Sanglier
};

enum class Pattern : std::uint8_t {
    Straight,
    Sine,
    Zigzag,
    Bounce,
    Dive,
    Wall,
    Formation
};

struct WaveData {
    float time = 0.f;
    EnemyType enemy = EnemyType::Ballon;
    Pattern pattern = Pattern::Straight;
    std::uint32_t count = 1;
    float spacing = 0.f;
};

struct BossData {
    std::string type;
    int health = 0;
    float time = 0.f;
};

struct LevelData {
    std::string name;
    std::string country;
    std::string owner;
    std::string background;
    std::vector<WaveData> waves;
    std::optional<BossData> boss;
};

const char *toString(EnemyType enemy) noexcept;
const char *toString(Pattern pattern) noexcept;

std::optional<EnemyType> enemyTypeFromString(const std::string &name);
std::optional<Pattern> patternFromString(const std::string &name);

std::string enemyTypeNames();
std::string patternNames();

}

#endif