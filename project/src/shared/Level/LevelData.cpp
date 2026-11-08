/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** LevelData.cpp
*/

#include "LevelData.hpp"
#include <array>

namespace r_type::level {

namespace {

template <class E>
struct Entry {
    const char *name;
    E value;
};

constexpr std::array<Entry<EnemyType>, 10> kEnemies{{
    {"ballon", EnemyType::Ballon},
    {"supporter", EnemyType::Supporter},
    {"defenseur", EnemyType::Defenseur},
    {"scorpion", EnemyType::Scorpion},
    {"harissa", EnemyType::Harissa},
    {"simit", EnemyType::Simit},
    {"cay", EnemyType::Cay},
    {"chat", EnemyType::Chat},
    {"grognard", EnemyType::Grognard},
    {"sanglier", EnemyType::Sanglier},
}};

constexpr std::array<Entry<Pattern>, 7> kPatterns{{
    {"straight", Pattern::Straight},
    {"sine", Pattern::Sine},
    {"zigzag", Pattern::Zigzag},
    {"bounce", Pattern::Bounce},
    {"dive", Pattern::Dive},
    {"wall", Pattern::Wall},
    {"formation", Pattern::Formation},
}};

template <class E, std::size_t N>
const char *nameOf(const std::array<Entry<E>, N> &table, E value) noexcept
{
    for (const auto &entry : table)
        if (entry.value == value)
            return entry.name;
    return "unknown";
}

template <class E, std::size_t N>
std::optional<E> valueOf(const std::array<Entry<E>, N> &table, const std::string &name)
{
    for (const auto &entry : table)
        if (name == entry.name)
            return entry.value;
    return std::nullopt;
}

template <class E, std::size_t N>
std::string namesOf(const std::array<Entry<E>, N> &table)
{
    std::string names;
    for (const auto &entry : table) {
        if (!names.empty())
            names += ", ";
        names += entry.name;
    }
    return names;
}

}

const char *toString(EnemyType enemy) noexcept { return nameOf(kEnemies, enemy); }
const char *toString(Pattern pattern) noexcept { return nameOf(kPatterns, pattern); }

std::optional<EnemyType> enemyTypeFromString(const std::string &name) { return valueOf(kEnemies, name); }
std::optional<Pattern> patternFromString(const std::string &name) { return valueOf(kPatterns, name); }

std::string enemyTypeNames() { return namesOf(kEnemies); }
std::string patternNames() { return namesOf(kPatterns); }

}