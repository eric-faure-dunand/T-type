/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Level.cpp
*/

#include "Level.hpp"

namespace r_type::game {

EnemyType enemyTypeFromName(const std::string &name)
{
    static const std::vector<std::pair<std::string, EnemyType>> names = {
        {"ballon", EnemyType::Ballon}, {"supporter", EnemyType::Supporter}, {"defenseur", EnemyType::Defenseur},
        {"scorpion", EnemyType::Scorpion}, {"harissa", EnemyType::Harissa}, {"simit", EnemyType::Simit},
        {"cay", EnemyType::Cay}, {"chat", EnemyType::Chat}, {"grognard", EnemyType::Grognard},
        {"sanglier", EnemyType::Sanglier},
    };
    for (const auto &[key, type] : names)
        if (key == name)
            return type;
    throw ::Error("Level: unknown enemy '" + name + "'.");
}

EnemyType bossTypeFromName(const std::string &name)
{
    static const std::vector<std::pair<std::string, EnemyType>> names = {
        {"eldiez", EnemyType::ElDiez}, {"couscoussier", EnemyType::Couscoussier}, {"doner", EnemyType::Doner},
        {"napoleon", EnemyType::Napoleon},
    };
    for (const auto &[key, type] : names)
        if (key == name)
            return type;
    throw ::Error("Level: unknown boss '" + name + "'.");
}

PatternType patternFromName(const std::string &name)
{
    static const std::vector<std::pair<std::string, PatternType>> names = {
        {"straight", PatternType::Straight}, {"sine", PatternType::Sine}, {"zigzag", PatternType::Zigzag},
        {"bounce", PatternType::Bounce}, {"dive", PatternType::Dive}, {"wall", PatternType::Wall},
        {"formation", PatternType::Formation},
    };
    for (const auto &[key, type] : names)
        if (key == name)
            return type;
    throw ::Error("Level: unknown pattern '" + name + "'.");
}

LevelData parseLevel(const std::string &text)
{
    try {
        const nlohmann::json json = nlohmann::json::parse(text);
        LevelData level;
        level.name = json.at("name").get<std::string>();
        level.country = json.at("country").get<std::string>();
        level.owner = json.value("owner", "");
        level.background = json.value("background", level.country);
        for (const nlohmann::json &wave : json.at("waves")) {
            WaveData data;
            data.time = wave.at("time").get<float>();
            data.enemy = enemyTypeFromName(wave.at("enemy").get<std::string>());
            data.pattern = patternFromName(wave.value("pattern", "straight"));
            data.count = wave.value("count", 1);
            data.spacing = wave.value("spacing", 0.f);
            if (data.time < 0.f || data.count < 1 || data.spacing < 0.f)
                throw ::Error("Level: invalid wave in '" + level.name + "'.");
            level.waves.push_back(data);
        }
        if (json.contains("boss")) {
            const nlohmann::json &boss = json.at("boss");
            level.boss = BossData{true, bossTypeFromName(boss.at("type").get<std::string>()), boss.value("health", 0),
                boss.value("time", 0.f)};
            if (level.boss.health < 0 || level.boss.time < 0.f)
                throw ::Error("Level: invalid boss in '" + level.name + "'.");
        }
        return level;
    } catch (const nlohmann::json::exception &e) {
        throw ::Error(std::string("Level: ") + e.what());
    }
}

LevelData loadLevel(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
        throw ::Error("Level: cannot open '" + path + "'.");
    const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return parseLevel(text);
}

std::vector<std::string> levelFiles()
{
    std::vector<std::string> files;
    std::error_code error;
    for (const auto &entry : std::filesystem::directory_iterator(kLevelDirectory, error))
        if (entry.is_regular_file() && entry.path().extension() == kLevelExtension)
            files.push_back(entry.path().generic_string());
    std::sort(files.begin(), files.end());
    if (files.size() > kMaxLevels)
        files.resize(kMaxLevels);
    return files;
}

std::string levelPath(std::size_t level)
{
    const std::vector<std::string> files = levelFiles();
    if (level >= files.size())
        throw ::Error("Level: unknown level " + std::to_string(level) + ".");
    return files[level];
}

}
