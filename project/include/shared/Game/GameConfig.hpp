/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** GameConfig.hpp
*/

#ifndef GAME_CONFIG_HPP
    #define GAME_CONFIG_HPP

namespace r_type::game {

constexpr float kWorldWidth = 1280.f;
constexpr float kWorldHeight = 720.f;
constexpr float kDespawnMargin = 100.f;

constexpr float kPlayerSpeed = 320.f;
constexpr float kPlayerWidth = 48.f;
constexpr float kPlayerHeight = 24.f;
constexpr int kPlayerHealth = 3;
constexpr float kPlayerFireRate = 6.f;
constexpr float kPlayerMissileSpeed = 700.f;

constexpr float kEnemySpeed = 160.f;
constexpr float kEnemySize = 40.f;
constexpr int kEnemyHealth = 2;
constexpr float kEnemySpawnInterval = 1.2f;
constexpr float kEnemyFireRate = 0.5f;
constexpr float kEnemyMissileSpeed = 320.f;

constexpr float kMissileWidth = 16.f;
constexpr float kMissileHeight = 6.f;

}

#endif
