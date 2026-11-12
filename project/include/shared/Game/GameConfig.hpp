/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** GameConfig.hpp
*/

#ifndef GAME_CONFIG_HPP
    #define GAME_CONFIG_HPP

    #include <array>

namespace r_type::game {

constexpr float kWorldWidth = 1280.f;
constexpr float kWorldHeight = 720.f;
constexpr float kDespawnMargin = 100.f;

constexpr int kCharacterCount = 4;

constexpr float kPlayerSpeed = 320.f;
constexpr float kPlayerWidth = 48.f;
constexpr float kPlayerHeight = 24.f;
constexpr int kPlayerHealth = 3;
constexpr float kPlayerFireRate = 6.f;
constexpr float kPlayerMissileSpeed = 700.f;

constexpr float kEnemySize = 40.f;
constexpr float kEnemyMissileSpeed = 320.f;

constexpr float kBossSize = 160.f;
constexpr float kBossMargin = 40.f;
constexpr float kBossVerticalSpeed = 55.f;
constexpr float kBossCannonTop = 0.2f;
constexpr float kBossCannonBottom = 0.8f;

struct EnemyStats {
    int health;
    float speed;
    float fireRate;
    float size;
};

constexpr std::array<EnemyStats, 14> kEnemyStats = {{
    {1, 170.f, 0.f, kEnemySize},
    {2, 120.f, 1.f / 2.6f, kEnemySize},
    {4, 70.f, 0.f, kEnemySize},
    {2, 140.f, 1.f / 3.f, kEnemySize},
    {1, 130.f, 0.f, kEnemySize},
    {1, 180.f, 0.f, kEnemySize},
    {2, 120.f, 1.f / 2.6f, kEnemySize},
    {1, 210.f, 0.f, kEnemySize},
    {2, 70.f, 1.f / 3.4f, kEnemySize},
    {3, 200.f, 0.f, kEnemySize},
    {50, 120.f, 0.9f, kBossSize},
    {50, 120.f, 0.8f, kBossSize},
    {40, 120.f, 1.f, kBossSize},
    {60, 120.f, 0.8f, kBossSize},
}};

constexpr float kSineAmplitude = 55.f;
constexpr float kSineFrequency = 3.f;
constexpr float kZigzagSpeed = 160.f;
constexpr float kZigzagRange = 70.f;
constexpr float kBounceSpeed = 210.f;
constexpr float kDiveSpeed = 120.f;
constexpr float kDiveBoost = 1.1f;
constexpr float kWallStop = 0.62f;
constexpr float kWallWait = 9.f;
constexpr float kFormationFrequency = 4.f;
constexpr float kFormationFast = 1.8f;
constexpr float kFormationSlow = 0.15f;
constexpr int kFormationRows = 5;
constexpr float kFormationSpacing = 60.f;
constexpr float kLevelRestartDelay = 10.f;

constexpr float kMissileWidth = 16.f;
constexpr float kMissileHeight = 6.f;

}

#endif
