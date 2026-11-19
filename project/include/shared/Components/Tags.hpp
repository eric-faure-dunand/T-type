/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Tags.hpp
*/

#ifndef COMPONENTS_TAGS_HPP
    #define COMPONENTS_TAGS_HPP

    #include <cstdint>

namespace r_type::game {

struct Player {
    int id = 1;
};

enum class EnemyType : std::uint8_t {
    Ballon = 0,
    Supporter = 1,
    Defenseur = 2,
    Scorpion = 3,
    Harissa = 4,
    Simit = 5,
    Cay = 6,
    Chat = 7,
    Grognard = 8,
    Sanglier = 9,
    ElDiez = 10,
    Couscoussier = 11,
    Doner = 12,
    Napoleon = 13
};

struct Enemy {
    EnemyType type = EnemyType::Ballon;
};

struct Boss {};

struct Projectile {
    bool fromPlayer = true;
};

enum class EntityKind : std::uint8_t {
    Player = 0,
    Enemy = 1,
    PlayerMissile = 2,
    EnemyMissile = 3
};

struct Kind {
    EntityKind value = EntityKind::Player;
};

}

#endif
