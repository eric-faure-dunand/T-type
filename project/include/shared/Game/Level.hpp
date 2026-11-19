/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Level.hpp
*/

#ifndef GAME_LEVEL_HPP
    #define GAME_LEVEL_HPP

    #include <algorithm>
    #include <filesystem>
    #include <fstream>
    #include <iterator>
    #include <string>
    #include <utility>
    #include <vector>

    #include <nlohmann/json.hpp>

    #include "Error.hpp"
    #include "GameConfig.hpp"
    #include "Pattern.hpp"
    #include "Tags.hpp"

namespace r_type::game {

struct WaveData {
    float time = 0.f;
    EnemyType enemy = EnemyType::Ballon;
    PatternType pattern = PatternType::Straight;
    int count = 1;
    float spacing = 0.f;
};

struct BossData {
    bool enabled = false;
    EnemyType type = EnemyType::ElDiez;
    int health = 0;
    float time = 0.f;
};

struct LevelData {
    std::string name;
    std::string country;
    std::string owner;
    std::string background;
    std::vector<WaveData> waves;
    BossData boss;
};

EnemyType enemyTypeFromName(const std::string &name);
EnemyType bossTypeFromName(const std::string &name);
PatternType patternFromName(const std::string &name);
LevelData parseLevel(const std::string &text);
LevelData loadLevel(const std::string &path);
std::vector<std::string> levelFiles();
std::string levelPath(std::size_t level);

}

#endif
