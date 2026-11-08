/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** LevelLoader.cpp
*/

#include "Level/LevelLoader.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <system_error>

#include <nlohmann/json.hpp>

namespace r_type::level {

namespace {

using Json = nlohmann::json;

[[noreturn]] void fail(const std::string &source, const std::string &path, const std::string &message)
{
    std::string text = source + ": ";
    if (!path.empty())
        text += path + ": ";
    throw LevelError(text + message);
}

std::string join(const std::string &prefix, const std::string &key)
{
    return prefix.empty() ? key : prefix + "." + key;
}

std::string format(double value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}

const Json &require(const Json &object, const std::string &prefix, const char *key,
    const std::string &source)
{
    const auto it = object.find(key);
    if (it == object.end())
        fail(source, prefix, std::string("missing field '") + key + "'");
    return *it;
}

std::string readString(const Json &object, const std::string &prefix, const char *key,
    const std::string &source, bool allowEmpty = false)
{
    const Json &value = require(object, prefix, key, source);
    const std::string path = join(prefix, key);
    if (!value.is_string())
        fail(source, path, "must be a string");
    std::string text = value.get<std::string>();
    if (text.empty() && !allowEmpty)
        fail(source, path, "must not be empty");
    return text;
}

float readNumber(const Json &object, const std::string &prefix, const char *key,
    const std::string &source, double min, double max)
{
    const Json &value = require(object, prefix, key, source);
    const std::string path = join(prefix, key);
    if (!value.is_number())
        fail(source, path, "must be a number");
    const double number = value.get<double>();
    if (!std::isfinite(number) || number < min || number > max)
        fail(source, path, "must be between " + format(min) + " and " + format(max));
    return static_cast<float>(number);
}

int readInteger(const Json &object, const std::string &prefix, const char *key,
    const std::string &source, int min, int max)
{
    const Json &value = require(object, prefix, key, source);
    const std::string path = join(prefix, key);
    if (!value.is_number_integer())
        fail(source, path, "must be an integer");
    const double number = value.get<double>();
    if (number < min || number > max)
        fail(source, path, "must be between " + std::to_string(min) + " and " + std::to_string(max));
    return static_cast<int>(number);
}

WaveData parseWave(const Json &json, const std::string &prefix, const std::string &source)
{
    if (!json.is_object())
        fail(source, prefix, "must be an object");

    WaveData wave;
    wave.time = readNumber(json, prefix, "time", source, 0.0, kMaxTimeSeconds);

    const std::string enemyName = readString(json, prefix, "enemy", source);
    const auto enemy = enemyTypeFromString(enemyName);
    if (!enemy)
        fail(source, join(prefix, "enemy"),
            "unknown enemy '" + enemyName + "' (expected one of: " + enemyTypeNames() + ")");
    wave.enemy = *enemy;

    const std::string patternName = readString(json, prefix, "pattern", source);
    const auto pattern = patternFromString(patternName);
    if (!pattern)
        fail(source, join(prefix, "pattern"),
            "unknown pattern '" + patternName + "' (expected one of: " + patternNames() + ")");
    wave.pattern = *pattern;

    wave.count = static_cast<std::uint32_t>(readInteger(json, prefix, "count", source, 1, kMaxCountPerWave));
    wave.spacing = readNumber(json, prefix, "spacing", source, 0.0, kMaxTimeSeconds);
    return wave;
}

BossData parseBoss(const Json &json, const std::string &prefix, const std::string &source)
{
    if (!json.is_object())
        fail(source, prefix, "must be an object");

    BossData boss;
    boss.type = readString(json, prefix, "type", source);
    boss.health = readInteger(json, prefix, "health", source, 1, kMaxBossHealth);
    boss.time = readNumber(json, prefix, "time", source, 0.0, kMaxTimeSeconds);
    return boss;
}

LevelData parseLevel(const Json &root, const std::string &source)
{
    if (!root.is_object())
        fail(source, "", "root must be a JSON object");

    LevelData level;
    level.name = readString(root, "", "name", source);
    level.country = readString(root, "", "country", source);
    level.background = readString(root, "", "background", source);

    if (root.contains("owner"))
        level.owner = readString(root, "", "owner", source, true);

    const std::string noPrefix;
    const Json &waves = require(root, noPrefix, "waves", source);
    if (!waves.is_array())
        fail(source, "waves", "must be an array");
    if (waves.size() > kMaxWaves)
        fail(source, "waves", "too many waves (" + std::to_string(waves.size())
            + ", maximum " + std::to_string(kMaxWaves) + ")");
    level.waves.reserve(waves.size());
    for (std::size_t i = 0; i < waves.size(); ++i)
        level.waves.push_back(parseWave(waves[i], "waves[" + std::to_string(i) + "]", source));

    std::stable_sort(level.waves.begin(), level.waves.end(),
        [](const WaveData &a, const WaveData &b) { return a.time < b.time; });

    const auto boss = root.find("boss");
    if (boss != root.end() && !boss->is_null())
        level.boss = parseBoss(*boss, "boss", source);

    return level;
}

}

LevelData loadFromString(const std::string &json, const std::string &sourceName)
{
    try {
        return parseLevel(Json::parse(json), sourceName);
    } catch (const LevelError &) {
        throw;
    } catch (const Json::exception &error) {
        fail(sourceName, "", std::string("invalid JSON: ") + error.what());
    }
}

LevelData loadFromFile(const std::filesystem::path &path)
{
    const std::string name = path.string();

    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec))
        fail(name, "", "cannot open file (not found or not a regular file)");

    const auto size = std::filesystem::file_size(path, ec);
    if (ec)
        fail(name, "", "cannot read the file size");
    if (size > kMaxFileBytes)
        fail(name, "", "file too large (" + std::to_string(size) + " bytes, maximum "
            + std::to_string(kMaxFileBytes) + ")");

    std::ifstream in(path, std::ios::binary);
    if (!in)
        fail(name, "", "cannot open file");
    std::string text(static_cast<std::size_t>(size), '\0');
    in.read(text.data(), static_cast<std::streamsize>(text.size()));
    if (!in)
        fail(name, "", "cannot read file");

    return loadFromString(text, name);
}

}